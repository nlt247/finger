#include "fingerprint_exports.h"
#include "fingerprint.h"
#include <string>

/**
 * @brief 创建一个指纹识别工作器实例
 * 
 * @param device_path 指纹设备的路径
 * @return FingerPrint_Worker* 指向新创建的指纹识别工作器实例的指针
 */
FingerPrint_Worker* CreateFingerPrintWorker(const char* device_path) {
    return new FingerPrint_Worker(device_path);
}

/**
 * @brief 销毁一个指纹识别工作器实例
 * 
 * @param worker 指向要销毁的指纹识别工作器实例的指针
 */
void DestroyFingerPrintWorker(FingerPrint_Worker* worker) {
    if (worker) {
        delete worker;
    }
}

/**
 * @brief 初始化指纹识别工作器
 * 
 * @param worker 指向指纹识别工作器实例的指针
 * @return bool 初始化成功返回 true，失败返回 false
 */
bool FP_Worker_Init(FingerPrint_Worker* worker) {
    if (!worker) return false;
    return worker->init();
}

/**
 * @brief 启动指纹识别工作器
 * 
 * @param worker 指向指纹识别工作器实例的指针
 */
void FP_Worker_Start(FingerPrint_Worker* worker) {
    if (worker) worker->start();
}

/**
 * @brief 停止指纹识别工作器
 * 
 * @param worker 指向指纹识别工作器实例的指针
 */
void FP_Worker_Stop(FingerPrint_Worker* worker) {
    if (worker) worker->stop();
}

/**
 * @brief 启动指纹检测
 * 
 * @param worker 指向指纹识别工作器实例的指针
 */
void FP_Worker_Detect_Start(FingerPrint_Worker* worker) {
    if (worker) worker->fp_detect_start();
}

/**
 * @brief 停止指纹检测
 * 
 * @param worker 指向指纹识别工作器实例的指针
 */
void FP_Worker_Detect_Stop(FingerPrint_Worker* worker) {
    if (worker) worker->fp_detect_stop();
}

/**
 * @brief 启动指纹录入
 * 
 * @param worker 指向指纹识别工作器实例的指针
 */
void FP_Worker_Enroll_Start(FingerPrint_Worker* worker) {
    if (worker) worker->fp_enroll_start();
}

/**
 * @brief 保存指纹模板
 * 
 * @param worker 指向指纹识别工作器实例的指针
 * @param finger_id 要保存的指纹的 ID
 */
void FP_Worker_Save(FingerPrint_Worker* worker, int finger_id) {
    if (worker) worker->fp_save(finger_id);
}

/**
 * @brief 删除指定 ID 的指纹模板
 * 
 * @param worker 指向指纹识别工作器实例的指针
 * @param finger_id 要删除的指纹的 ID
 */
void FP_Worker_Delete(FingerPrint_Worker* worker, int finger_id) {
    if (worker) worker->fp_delete(finger_id);
}

/**
 * @brief 检查指纹识别工作器是否在线
 * 
 * @param worker 指向指纹识别工作器实例的指针
 * @return bool 在线返回 true，离线返回 false
 */
bool FP_Worker_Online(FingerPrint_Worker* worker) {
    if (!worker) return false;
    return worker->online();
}

/**
 * @brief 获取可用的指纹 ID
 * 
 * @param worker 指向指纹识别工作器实例的指针
 * @return int 可用的指纹 ID，如果没有可用 ID 或工作器指针为空则返回 -1
 */
int FP_Worker_Avail_ID(FingerPrint_Worker* worker) {
    if (!worker) return -1;
    return worker->avail_id();
}

/**
 * @brief 检查是否有新的指纹模板可用
 * 
 * @param worker 指向指纹识别工作器实例的指针
 * @return bool 有新模板可用返回 true，否则返回 false
 */
bool FP_Worker_New_Template_Avail(FingerPrint_Worker* worker) {
    if (!worker) return false;
    return worker->new_template_avail();
}