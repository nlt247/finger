#include "fingerprint_api.h"
#include "fingerprint.h"
#include <thread>
#include <mutex>
#include <memory>

// 全局变量
static std::unique_ptr<FingerPrint_Worker> worker = nullptr;
static fp_callback user_callback = nullptr;
static void* user_data = nullptr;
static std::mutex mutex;

// API实现
FINGERPRINT_API int fp_init(const char* device_path) {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (!worker) {
        worker.reset(new FingerPrint_Worker(device_path));
        
        worker->setDetectionCallback([](int id) {
            if (user_callback) {
                user_callback(FP_STATUS_FINGER_DETECTED, id, user_data);
            }
        });
        
        worker->setMessageCallback([](uint32_t proc, uint32_t step) {
            if (user_callback) {
                user_callback(proc, step, user_data);
            }
        });
        
        worker->start();
        
        // 等待初始化完成
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
        if (worker->online()) {
            return FP_SUCCESS;
        } else {
            return FP_ERROR_INIT_FAILED;
        }
    }
    
    return worker->online() ? FP_SUCCESS : FP_ERROR_INIT_FAILED;
}

FINGERPRINT_API void fp_cleanup() {
    std::lock_guard<std::mutex> lock(mutex);
    
    if (worker) {
        worker->stop();
        worker.reset();
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
    
    worker->fp_detect_start();
    return FP_SUCCESS;
}

FINGERPRINT_API int fp_stop_detection() {
    if (!worker || !worker->online()) return FP_ERROR_DEVICE_NOT_FOUND;
    
    worker->fp_detect_stop();
    return FP_SUCCESS;
}

FINGERPRINT_API int fp_start_enrollment(fp_callback callback, void* data) {
    if (!worker || !worker->online()) return FP_ERROR_DEVICE_NOT_FOUND;
    
    user_callback = callback;
    user_data = data;
    
    worker->fp_enroll_start();
    return FP_SUCCESS;
}

FINGERPRINT_API int fp_save_template(int template_id) {
    if (!worker || !worker->online()) return FP_ERROR_DEVICE_NOT_FOUND;
    
    if (!worker->new_template_avail()) return FP_ERROR_TEMPLATE_INVALID;
    
    worker->fp_save(template_id);
    return FP_SUCCESS;
}

FINGERPRINT_API int fp_delete_template(int template_id) {
    if (!worker || !worker->online()) return FP_ERROR_DEVICE_NOT_FOUND;
    
    worker->fp_delete(template_id);
    return FP_SUCCESS;
}

FINGERPRINT_API int fp_get_available_id() {
    if (!worker || !worker->online()) return -1;
    
    return worker->avail_id();
}