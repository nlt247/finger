#include "fingerprint_api.h"
#include "fingerprint.h"
#include "communication.h"
#include "command.h"
#include "device.h"

#include <QCoreApplication>
#include <QThread>
#include <QString>
#include <QMutex>
#include <memory>

// 全局变量
static int argc = 1;
static char *argv[] = {(char*)"fingerprint_lib"};
static QCoreApplication *app = nullptr;
static std::unique_ptr<FingerPrint_Worker> worker = nullptr;
static QThread workerThread;
static fp_callback user_callback = nullptr;
static void* user_data = nullptr;
static QMutex mutex;

// 初始化Qt应用程序
static void ensure_qt_initialized() {
    if (!QCoreApplication::instance()) {
        app = new QCoreApplication(argc, argv);
    }
}

// 回调函数转发
class CallbackHandler : public QObject {
    Q_OBJECT
public:
    CallbackHandler() {}

public slots:
    void handleDetection(int id) {
        if (user_callback) {
            user_callback(FP_STATUS_FINGER_DETECTED, id, user_data);
        }
    }
    
    void handleMessage(uint32_t proc, uint32_t step) {
        if (user_callback) {
            user_callback(proc, step, user_data);
        }
    }
};

static CallbackHandler *callback_handler = nullptr;

// API实现
FINGERPRINT_API int fp_init(const char* device_path) {
    QMutexLocker locker(&mutex);
    
    ensure_qt_initialized();
    
    if (!worker) {
        worker.reset(new FingerPrint_Worker(QString(device_path)));
        worker->moveToThread(&workerThread);
        
        callback_handler = new CallbackHandler();
        
        QObject::connect(&workerThread, &QThread::finished, worker.get(), &QObject::deleteLater);
        QObject::connect(worker.get(), &FingerPrint_Worker::fp_dected, 
                         callback_handler, &CallbackHandler::handleDetection);
        QObject::connect(worker.get(), &FingerPrint_Worker::fp_message, 
                         callback_handler, &CallbackHandler::handleMessage);
        
        workerThread.start();
        
        // 启动工作线程
        QMetaObject::invokeMethod(worker.get(), "start", Qt::QueuedConnection);
        
        // 等待初始化完成
        QThread::msleep(1000);
        
        if (worker->online()) {
            return FP_SUCCESS;
        } else {
            return FP_ERROR_INIT_FAILED;
        }
    }
    
    return worker->online() ? FP_SUCCESS : FP_ERROR_INIT_FAILED;
}

FINGERPRINT_API void fp_cleanup() {
    QMutexLocker locker(&mutex);
    
    if (worker) {
        QMetaObject::invokeMethod(worker.get(), "stop", Qt::QueuedConnection);
        workerThread.quit();
        workerThread.wait();
        worker.reset();
        
        delete callback_handler;
        callback_handler = nullptr;
    }
    
    if (app) {
        delete app;
        app = nullptr;
    }
}

FINGERPRINT_API int fp_get_status() {
    if (!worker) return FP_STATUS_ERROR;
    
    return worker->online() ? FP_STATUS_READY : FP_STATUS_ERROR;
}

FINGERPRINT_API int fp_start_detection(fp_callback callback, void* data) {
    if (!worker || !worker->online()) return FP_ERROR_DEVICE_NOT_FOUND;
    
    user_callback = callback;
    user_data = data;
    
    QMetaObject::invokeMethod(worker.get(), "fp_detect_start", Qt::QueuedConnection);
    return FP_SUCCESS;
}

FINGERPRINT_API int fp_stop_detection() {
    if (!worker || !worker->online()) return FP_ERROR_DEVICE_NOT_FOUND;
    
    QMetaObject::invokeMethod(worker.get(), "fp_detect_stop", Qt::QueuedConnection);
    return FP_SUCCESS;
}

FINGERPRINT_API int fp_start_enrollment(fp_callback callback, void* data) {
    if (!worker || !worker->online()) return FP_ERROR_DEVICE_NOT_FOUND;
    
    user_callback = callback;
    user_data = data;
    
    QMetaObject::invokeMethod(worker.get(), "fp_enroll_start", Qt::QueuedConnection);
    return FP_SUCCESS;
}

FINGERPRINT_API int fp_save_template(int template_id) {
    if (!worker || !worker->online()) return FP_ERROR_DEVICE_NOT_FOUND;
    
    if (!worker->new_template_avail()) return FP_ERROR_TEMPLATE_INVALID;
    
    QMetaObject::invokeMethod(worker.get(), "fp_save", Qt::QueuedConnection, 
                             Q_ARG(uint32_t, template_id));
    return FP_SUCCESS;
}

FINGERPRINT_API int fp_delete_template(int template_id) {
    if (!worker || !worker->online()) return FP_ERROR_DEVICE_NOT_FOUND;
    
    QMetaObject::invokeMethod(worker.get(), "fp_delete", Qt::QueuedConnection, 
                             Q_ARG(uint32_t, template_id));
    return FP_SUCCESS;
}

FINGERPRINT_API int fp_get_available_id() {
    if (!worker || !worker->online()) return -1;
    
    return worker->avail_id();
}

#include "fingerprint_api.moc"