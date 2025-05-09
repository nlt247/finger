#include "fingerprint_api.h"
#include "fingerprint.h"
#include <thread>
#include <mutex>
#include <memory>

// 全局变量
// 用于管理指纹工作线程的唯一指针
static std::unique_ptr<FingerPrint_Worker> worker = nullptr;
// 用户定义的回调函数指针
static fp_callback user_callback = nullptr;
// 用户传递的数据指针
static void* user_data = nullptr;
// 用于线程同步的互斥锁
static std::mutex mutex;

/**
 * @brief 初始化指纹设备
 * 
 * 该函数用于初始化指纹设备，创建指纹工作线程并设置回调函数。
 * 等待1秒后检查设备是否在线，根据结果返回相应的状态码。
 * 
 * @param device_path 指纹设备的路径
 * @return int 初始化结果，FP_SUCCESS 表示成功，FP_ERROR_INIT_FAILED 表示失败
 */
FINGERPRINT_API int fp_init(const char* device_path) {
    // 加锁以保证线程安全
    std::lock_guard<std::mutex> lock(mutex);
    
    // 如果工作线程未创建，则创建并初始化
    if (!worker) {
        // 重置智能指针，创建新的指纹工作线程
        worker.reset(new FingerPrint_Worker(device_path));
        
        // 设置指纹检测回调函数
        worker->setDetectionCallback([](int id) {
            // 如果用户提供了回调函数，则调用该函数
            if (user_callback) {
                user_callback(FP_STATUS_FINGER_DETECTED, id, user_data);
            }
        });
        
        // 设置消息回调函数
        worker->setMessageCallback([](uint32_t proc, uint32_t step) {
            // 如果用户提供了回调函数，则调用该函数
            if (user_callback) {
                user_callback(proc, step, user_data);
            }
        });
        
        // 启动指纹工作线程
        worker->start();
        
        // 等待初始化完成，睡眠1秒
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        
        // 检查设备是否在线
        if (worker->online()) {
            return FP_SUCCESS;
        } else {
            return FP_ERROR_INIT_FAILED;
        }
    }
    
    // 如果工作线程已存在，直接检查设备是否在线
    return worker->online() ? FP_SUCCESS : FP_ERROR_INIT_FAILED;
}

/**
 * @brief 清理指纹设备资源
 * 该函数用于停止指纹工作线程并释放相关资源。
 */
FINGERPRINT_API void fp_cleanup() {
    // 加锁以保证线程安全
    std::lock_guard<std::mutex> lock(mutex);
    
    // 如果工作线程存在，则停止并释放资源
    if (worker) {
        worker->stop();
        worker.reset();
    }
}

/**
 * @brief 获取指纹设备状态
 * 该函数用于检查指纹设备是否在线，并返回相应的状态码。
 * @return int 设备状态，FP_STATUS_READY 表示就绪，FP_STATUS_ERROR 表示错误
 */
FINGERPRINT_API int fp_get_status() {
    // 如果工作线程不存在，返回错误状态
    if (!worker) return FP_STATUS_ERROR;
    
    // 根据设备是否在线返回相应状态
    return worker->online() ? FP_STATUS_READY : FP_STATUS_ERROR;
}

/**
 * @brief 开始指纹检测
 * 该函数用于启动指纹检测功能，并设置用户回调函数和数据。
 * @param callback 用户定义的回调函数
 * @param data 用户传递的数据指针
 * @return int 操作结果，FP_SUCCESS 表示成功，FP_ERROR_DEVICE_NOT_FOUND 表示设备未找到
 */
FINGERPRINT_API int fp_start_detection(fp_callback callback, void* data) {
    // 如果工作线程不存在或设备未在线，返回设备未找到错误
    if (!worker || !worker->online()) return FP_ERROR_DEVICE_NOT_FOUND;
    
    // 设置用户回调函数和数据
    user_callback = callback;
    user_data = data;
    
    // 启动指纹检测
    worker->fp_detect_start();
    return FP_SUCCESS;
}

/**
 * @brief 停止指纹检测
 * 该函数用于停止正在进行的指纹检测功能。
 * @return int 操作结果，FP_SUCCESS 表示成功，FP_ERROR_DEVICE_NOT_FOUND 表示设备未找到
 */
FINGERPRINT_API int fp_stop_detection() {
    // 如果工作线程不存在或设备未在线，返回设备未找到错误
    if (!worker || !worker->online()) return FP_ERROR_DEVICE_NOT_FOUND;
    
    // 停止指纹检测
    worker->fp_detect_stop();
    return FP_SUCCESS;
}

/**
 * @brief 开始指纹录入
 * 该函数用于启动指纹录入功能，并设置用户回调函数和数据。
 * @param callback 用户定义的回调函数
 * @param data 用户传递的数据指针
 * @return int 操作结果，FP_SUCCESS 表示成功，FP_ERROR_DEVICE_NOT_FOUND 表示设备未找到
 */
FINGERPRINT_API int fp_start_enrollment(fp_callback callback, void* data) {
    // 如果工作线程不存在或设备未在线，返回设备未找到错误
    if (!worker || !worker->online()) return FP_ERROR_DEVICE_NOT_FOUND;
    
    // 设置用户回调函数和数据
    user_callback = callback;
    user_data = data;
    
    // 启动指纹录入
    worker->fp_enroll_start();
    return FP_SUCCESS;
}

/**
 * @brief 保存指纹模板
 * 该函数用于保存新的指纹模板到指定的ID。
 * @param template_id 要保存模板的ID
 * @return int 操作结果，FP_SUCCESS 表示成功，FP_ERROR_DEVICE_NOT_FOUND 表示设备未找到，
 *             FP_ERROR_TEMPLATE_INVALID 表示模板无效
 */
FINGERPRINT_API int fp_save_template(int template_id) {
    // 如果工作线程不存在或设备未在线，返回设备未找到错误
    if (!worker || !worker->online()) return FP_ERROR_DEVICE_NOT_FOUND;
    
    // 如果没有可用的新模板，返回模板无效错误
    if (!worker->new_template_avail()) return FP_ERROR_TEMPLATE_INVALID;
    
    // 保存指纹模板到指定ID
    worker->fp_save(template_id);
    return FP_SUCCESS;
}

/**
 * @brief 删除指纹模板
 * 该函数用于删除指定ID的指纹模板。
 * @param template_id 要删除模板的ID
 * @return int 操作结果，FP_SUCCESS 表示成功，FP_ERROR_DEVICE_NOT_FOUND 表示设备未找到
 */
FINGERPRINT_API int fp_delete_template(int template_id) {
    // 如果工作线程不存在或设备未在线，返回设备未找到错误
    if (!worker || !worker->online()) return FP_ERROR_DEVICE_NOT_FOUND;
    
    // 删除指定ID的指纹模板
    worker->fp_delete(template_id);
    return FP_SUCCESS;
}

/**
 * @brief 获取可用的指纹模板ID
 * 该函数用于获取当前可用的指纹模板ID。
 * @return int 可用的模板ID，如果设备未在线或无可用ID，返回 -1
 */
FINGERPRINT_API int fp_get_available_id() {
    // 如果工作线程不存在或设备未在线，返回 -1
    if (!worker || !worker->online()) return -1;
    
    // 获取可用的模板ID
    return worker->avail_id();
}

FINGERPRINT_API int test_add(int a, int b) {
    return a + b;
}

/**
 * @brief 删除指定 ID 的指纹模板
 * @param id 要删除的指纹 ID
 * @return 删除操作的返回值
 */
 FINGERPRINT_API int fp_remove_template(int id)
 {
     if (!worker || !worker->online()) return FP_ERROR_DEVICE_NOT_FOUND;
     
     worker->remove(id);
     return FP_SUCCESS;
 }
 
 /**
  * @brief 存储指纹模板到指定 ID
  * @param id 要存储的指纹 ID
  * @return 存储操作的返回值
  */
 FINGERPRINT_API int fp_store(int id)
 {
     if (!worker || !worker->online()) return FP_ERROR_DEVICE_NOT_FOUND;
 
     worker->store(id);
     return FP_SUCCESS;
 }
 
 /**
 * @brief 生成指纹模板
 * @param step 生成步骤
 * @return 生成操作的返回值
 */
 FINGERPRINT_API int fp_generate(int step)
 {
     if (!worker || !worker->online()) return FP_ERROR_DEVICE_NOT_FOUND;
 
     return worker->generate(step);
 }
 
 /**
 * @brief 更新可用的指纹 ID
 */
 FINGERPRINT_API void fp_update_avail_id()
 {
     if (!worker || !worker->online()) return;
     
     worker->update_avail_id();
 }
 
 /**
 * @brief 捕获指纹图像
 * @return 捕获操作的返回值
 */
 FINGERPRINT_API int fp_capture()
 {
     if (!worker || !worker->online()) return FP_ERROR_DEVICE_NOT_FOUND;
 
     return worker->capture();
 }
 
 /**
 * @brief 检测手指是否存在
 * @return 存在返回 true，不存在返回 false
 */
 FINGERPRINT_API bool fp_detect_finger()
 {
     return worker->detect_finger();
 }

FINGERPRINT_API int fp_get_detect_finger() 
{
    return worker->get_detect_finger();
}
 
 /**
 * @brief 搜索匹配的指纹 ID
 * @param id 用于存储匹配到的指纹 ID
 * @return 搜索成功返回 true，失败返回 false
 */
 FINGERPRINT_API int fp_search_get_id() {
    if (!worker || !worker->online()) return -1;
    
    int id = -1;
    bool success = worker->search(&id);
    return success ? id : -1;
}
 
 /**
 * @brief 合并指纹模板
 * @param step 合并步骤
 * @return 合并操作的返回值
 */
 FINGERPRINT_API int fp_merge(int step)
 {
     if (!worker || !worker->online()) return FP_ERROR_DEVICE_NOT_FOUND;
     
     return worker->merge(step);
 }