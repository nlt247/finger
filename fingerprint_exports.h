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
FPAPI int fp_init_connection(const char* devname);
FPAPI void fp_close_connection();

// 指纹操作
FPAPI int fp_get_image();
FPAPI int fp_finger_detect(int* detect_result);
FPAPI int fp_store_char(int tmpl_no, int ram_buffer_id, int* dup_tmpl_no);
FPAPI int fp_del_char(int start_tmpl_no, int end_tmpl_no);
FPAPI int fp_get_empty_id(int start_tmpl_no, int end_tmpl_no, int* empty_id);
FPAPI int fp_generate(int ram_buffer_id);
FPAPI int fp_merge(int ram_buffer_id, int merge_count);
FPAPI int fp_search(int ram_buffer_id, int start_id, int search_count, 
                   int* tmpl_no, int* learn_result);

#ifdef __cplusplus
}
#endif

#endif // __FP_EXPORTS_H__