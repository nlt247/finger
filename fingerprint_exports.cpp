#include "fingerprint_exports.h"
#include "communication.h"
#include <memory>

// 全局通信对象
static std::unique_ptr<CCommunication> g_pCommunication = nullptr;

// 初始化和连接管理
int fp_init_connection(const char* devname) {
    if (g_pCommunication) {
        g_pCommunication->CloseConnection();
        g_pCommunication.reset();
    }
    
    // 在C++11中使用new和reset代替make_unique
    g_pCommunication.reset(new CCommunication(devname));
    return g_pCommunication->Run_InitConnection();
}

void fp_close_connection() {
    if (g_pCommunication) {
        g_pCommunication->CloseConnection();
        g_pCommunication.reset();
    }
}

// 指纹操作
int fp_get_image() {
    if (!g_pCommunication) {
        return ERR_CONNECTION;
    }
    return g_pCommunication->Run_GetImage();
}

int fp_finger_detect(int* detect_result) {
    if (!g_pCommunication) {
        return ERR_CONNECTION;
    }
    return g_pCommunication->Run_FingerDetect(detect_result);
}

int fp_store_char(int tmpl_no, int ram_buffer_id, int* dup_tmpl_no) {
    if (!g_pCommunication) {
        return ERR_CONNECTION;
    }
    return g_pCommunication->Run_StoreChar(tmpl_no, ram_buffer_id, dup_tmpl_no);
}

int fp_del_char(int start_tmpl_no, int end_tmpl_no) {
    if (!g_pCommunication) {
        return ERR_CONNECTION;
    }
    return g_pCommunication->Run_DelChar(start_tmpl_no, end_tmpl_no);
}

int fp_get_empty_id(int start_tmpl_no, int end_tmpl_no, int* empty_id) {
    if (!g_pCommunication) {
        return ERR_CONNECTION;
    }
    return g_pCommunication->Run_GetEmptyID(start_tmpl_no, end_tmpl_no, empty_id);
}

int fp_generate(int ram_buffer_id) {
    if (!g_pCommunication) {
        return ERR_CONNECTION;
    }
    return g_pCommunication->Run_Generate(ram_buffer_id);
}

int fp_merge(int ram_buffer_id, int merge_count) {
    if (!g_pCommunication) {
        return ERR_CONNECTION;
    }
    return g_pCommunication->Run_Merge(ram_buffer_id, merge_count);
}

int fp_search(int ram_buffer_id, int start_id, int search_count, 
             int* tmpl_no, int* learn_result) {
    if (!g_pCommunication) {
        return ERR_CONNECTION;
    }
    return g_pCommunication->Run_Search(ram_buffer_id, start_id, search_count, tmpl_no, learn_result);
}