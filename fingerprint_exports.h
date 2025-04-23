#ifndef __FP_EXPORTS_H__
#define __FP_EXPORTS_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
  #ifdef BUILDING_DLL
    #define FPAPI __declspec(dllexport)
  #else
    #define FPAPI __declspec(dllimport)
  #endif
#else
  #define FPAPI __attribute__((visibility("default")))
#endif

// 初始化和连接管理
FPAPI int fp_init_connection(const char* dev_name);
FPAPI int fp_open_connection();
FPAPI void fp_close_connection();

// 指纹操作
FPAPI int fp_get_image();
FPAPI int fp_finger_detect();
FPAPI int fp_store_char(int id);
FPAPI int fp_del_char(int id);
FPAPI int update_avail_id();
FPAPI int fp_get_empty_id();
FPAPI int fp_generate(int ram_buffer_id);
FPAPI int fp_merge(int ram_buffer_id);
FPAPI int fp_search(int *id);

#ifdef __cplusplus
}
#endif

#endif // __FP_EXPORTS_H__