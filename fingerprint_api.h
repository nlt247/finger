#ifndef FINGERPRINT_API_H
#define FINGERPRINT_API_H

#ifdef __cplusplus
extern "C" {
#endif

// 导出符号宏定义
#if defined(_WIN32) || defined(__CYGWIN__)
  #ifdef BUILDING_DLL
    #define FINGERPRINT_API __declspec(dllexport)
  #else
    #define FINGERPRINT_API __declspec(dllimport)
  #endif
#else
  #ifdef BUILDING_DLL
    #define FINGERPRINT_API __attribute__((visibility("default")))
  #else
    #define FINGERPRINT_API
  #endif
#endif

// 错误码定义
typedef enum {
    FP_SUCCESS = 0,
    FP_ERROR_INIT_FAILED = -1,
    FP_ERROR_CAPTURE_FAILED = -2,
    FP_ERROR_PROCESS_FAILED = -3,
    FP_ERROR_TEMPLATE_INVALID = -4,
    FP_ERROR_DEVICE_NOT_FOUND = -5,
    FP_ERROR_COMMUNICATION = -6
} FP_ERROR_CODE;

// 指纹状态
typedef enum {
    FP_STATUS_READY = 0,
    FP_STATUS_FINGER_DETECTED = 1,
    FP_STATUS_NO_FINGER = 2,
    FP_STATUS_BUSY = 3,
    FP_STATUS_ERROR = 4
} FP_STATUS;

// 注册过程中的阶段
typedef enum {
    FP_ENROLL_NONE = 0,
    FP_ENROLL_PLACE_FINGER = 1,
    FP_ENROLL_REMOVE_FINGER = 2,
    FP_ENROLL_CAPTURE = 3,
    FP_ENROLL_GENERATE = 4,
    FP_ENROLL_MERGE = 5,
    FP_ENROLL_COMPLETE = 6,
    FP_ENROLL_DUPLICATE = 7
} FP_ENROLL_STAGE;

// 指纹操作回调函数类型
typedef void (*fp_callback)(int status, int template_id, void* user_data);

// 初始化指纹库
FINGERPRINT_API int fp_init(const char* device_path);

// 关闭并清理资源
FINGERPRINT_API void fp_cleanup();

// 获取指纹设备状态
FINGERPRINT_API int fp_get_status();

// 启动指纹检测
FINGERPRINT_API int fp_start_detection(fp_callback callback, void* user_data);

// 停止指纹检测
FINGERPRINT_API int fp_stop_detection();

// 启动指纹注册过程
FINGERPRINT_API int fp_start_enrollment(fp_callback callback, void* user_data);

// 保存当前模板到指定ID
FINGERPRINT_API int fp_save_template(int template_id);

// 删除指定ID的指纹模板
FINGERPRINT_API int fp_delete_template(int template_id);

// 获取当前可用的ID
FINGERPRINT_API int fp_get_available_id();

FINGERPRINT_API int test_add(int a, int b);

FINGERPRINT_API int remove(int id);

FINGERPRINT_API int store(int id);

FINGERPRINT_API int generate(int step);

FINGERPRINT_API void update_avail_id();

FINGERPRINT_API int capture();

FINGERPRINT_API bool detect_finger();

FINGERPRINT_API bool search(int* id);

FINGERPRINT_API int merge(int step);

#ifdef __cplusplus
}
#endif

#endif // FINGERPRINT_API_H