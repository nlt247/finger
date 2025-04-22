#include "fingerprint_exports.h"
#include "communication.h"
#include <memory>

// 全局通信对象
static std::unique_ptr<CCommunication> g_pCommunication = nullptr;

/**
 * @brief 初始化并连接指纹设备
 * 
 * 若全局通信对象已存在，则先关闭连接并重置该对象。
 * 然后使用指定的设备名称创建新的通信对象，并初始化连接。
 * 
 * @param devname 设备名称
 * @return int 连接初始化结果，由 CCommunication::Run_InitConnection() 返回
 */
int fp_init_connection(const char* devname) {
    if (g_pCommunication) {
        g_pCommunication->CloseConnection();
        g_pCommunication.reset();
    }
    
    g_pCommunication = std::make_unique<CCommunication>(devname);
    return g_pCommunication->Run_InitConnection();
}

/**
 * @brief 关闭指纹设备的连接
 * 
 * 若全局通信对象存在，则关闭其连接并重置该对象。
 */
void fp_close_connection() {
    if (g_pCommunication) {
        g_pCommunication->CloseConnection();
        g_pCommunication.reset();
    }
}

/**
 * @brief 获取指纹图像
 * 
 * 若全局通信对象不存在，返回连接错误码。
 * 否则，调用通信对象的方法获取指纹图像。
 * 
 * @return int 获取指纹图像的结果，由 CCommunication::Run_GetImage() 返回
 */
int fp_get_image() {
    if (!g_pCommunication) {
        return ERR_CONNECTION;
    }
    return g_pCommunication->Run_GetImage();
}

/**
 * @brief 检测指纹是否存在
 * 
 * 若全局通信对象不存在，返回连接错误码。
 * 否则，调用通信对象的方法检测指纹，并将结果存储在 detect_result 中。
 * 
 * @param detect_result 用于存储指纹检测结果的指针
 * @return int 指纹检测操作的结果，由 CCommunication::Run_FingerDetect() 返回
 */
int fp_finger_detect(int* detect_result) {
    if (!g_pCommunication) {
        return ERR_CONNECTION;
    }
    return g_pCommunication->Run_FingerDetect(detect_result);
}

/**
 * @brief 存储指纹特征模板
 * 
 * 若全局通信对象不存在，返回连接错误码。
 * 否则，调用通信对象的方法存储指纹特征模板，并将重复模板编号存储在 dup_tmpl_no 中。
 * 
 * @param tmpl_no 要存储的模板编号
 * @param ram_buffer_id 内存缓冲区 ID
 * @param dup_tmpl_no 用于存储重复模板编号的指针
 * @return int 存储操作的结果，由 CCommunication::Run_StoreChar() 返回
 */
int fp_store_char(int tmpl_no, int ram_buffer_id, int* dup_tmpl_no) {
    if (!g_pCommunication) {
        return ERR_CONNECTION;
    }
    return g_pCommunication->Run_StoreChar(tmpl_no, ram_buffer_id, dup_tmpl_no);
}

/**
 * @brief 删除指纹特征模板
 * 
 * 若全局通信对象不存在，返回连接错误码。
 * 否则，调用通信对象的方法删除指定范围内的指纹特征模板。
 * 
 * @param start_tmpl_no 要删除的起始模板编号
 * @param end_tmpl_no 要删除的结束模板编号
 * @return int 删除操作的结果，由 CCommunication::Run_DelChar() 返回
 */
int fp_del_char(int start_tmpl_no, int end_tmpl_no) {
    if (!g_pCommunication) {
        return ERR_CONNECTION;
    }
    return g_pCommunication->Run_DelChar(start_tmpl_no, end_tmpl_no);
}

/**
 * @brief 获取空闲的模板编号
 * 
 * 若全局通信对象不存在，返回连接错误码。
 * 否则，调用通信对象的方法获取指定范围内的空闲模板编号，并将结果存储在 empty_id 中。
 * 
 * @param start_tmpl_no 查找的起始模板编号
 * @param end_tmpl_no 查找的结束模板编号
 * @param empty_id 用于存储空闲模板编号的指针
 * @return int 获取空闲模板编号操作的结果，由 CCommunication::Run_GetEmptyID() 返回
 */
int fp_get_empty_id(int start_tmpl_no, int end_tmpl_no, int* empty_id) {
    if (!g_pCommunication) {
        return ERR_CONNECTION;
    }
    return g_pCommunication->Run_GetEmptyID(start_tmpl_no, end_tmpl_no, empty_id);
}

/**
 * @brief 生成指纹特征
 * 
 * 若全局通信对象不存在，返回连接错误码。
 * 否则，调用通信对象的方法在指定内存缓冲区中生成指纹特征。
 * 
 * @param ram_buffer_id 内存缓冲区 ID
 * @return int 生成指纹特征操作的结果，由 CCommunication::Run_Generate() 返回
 */
int fp_generate(int ram_buffer_id) {
    if (!g_pCommunication) {
        return ERR_CONNECTION;
    }
    return g_pCommunication->Run_Generate(ram_buffer_id);
}

/**
 * @brief 合并指纹特征
 * 
 * 若全局通信对象不存在，返回连接错误码。
 * 否则，调用通信对象的方法在指定内存缓冲区中合并指定数量的指纹特征。
 * 
 * @param ram_buffer_id 内存缓冲区 ID
 * @param merge_count 要合并的指纹特征数量
 * @return int 合并指纹特征操作的结果，由 CCommunication::Run_Merge() 返回
 */
int fp_merge(int ram_buffer_id, int merge_count) {
    if (!g_pCommunication) {
        return ERR_CONNECTION;
    }
    return g_pCommunication->Run_Merge(ram_buffer_id, merge_count);
}

/**
 * @brief 搜索指纹特征模板
 * 
 * 若全局通信对象不存在，返回连接错误码。
 * 否则，调用通信对象的方法在指定范围内搜索指纹特征模板，并将匹配的模板编号和学习结果存储在 tmpl_no 和 learn_result 中。
 * 
 * @param ram_buffer_id 内存缓冲区 ID
 * @param start_id 搜索的起始模板编号
 * @param search_count 要搜索的模板数量
 * @param tmpl_no 用于存储匹配的模板编号的指针
 * @param learn_result 用于存储学习结果的指针
 * @return int 搜索操作的结果，由 CCommunication::Run_Search() 返回
 */
int fp_search(int ram_buffer_id, int start_id, int search_count, 
             int* tmpl_no, int* learn_result) {
    if (!g_pCommunication) {
        return ERR_CONNECTION;
    }
    return g_pCommunication->Run_Search(ram_buffer_id, start_id, search_count, tmpl_no, learn_result);
}