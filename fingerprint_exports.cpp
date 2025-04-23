#include "fingerprint_exports.h"
#include "communication.h"
#include <memory>

#if __cplusplus <= 201103L
namespace std {
    template <typename T, typename... Args>
    std::unique_ptr<T> make_unique(Args&&... args) {
        return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
    }
}
#endif

// 全局通信对象
std::unique_ptr<CCommunication> m_comm;

// 初始化和连接管理
int fp_init_connection(const char* dev_name) {
    if (!m_comm){
        m_comm = std::make_unique<CCommunication>(dev_name);
    }

	if (m_comm->Run_InitConnection() != CONNECTION_SUCCESS)
		return false;
	return true;
}

void fp_close_connection() {
    m_comm->CloseConnection();
}

// 指纹操作
int fp_get_image() {
    return m_comm->Run_GetImage();
}

int fp_finger_detect() {
    int ret_code;

	if (m_comm->Run_FingerDetect(&ret_code) != ERR_SUCCESS)
		return false;

	return ret_code;
}

int fp_store_char(int id) {
    int dup_id, ret_code;

	fp_del_char(id);
	ret_code = m_comm->Run_StoreChar(id, 0, &dup_id);
	update_avail_id();

	return ret_code;
}

int fp_del_char(int id) {
    int ret_code;

	ret_code = m_comm->Run_DelChar(id, id);
	update_avail_id();

	return ret_code;
}

int update_avail_id() {
    int empty_id;

	if (m_comm->Run_GetEmptyID(1, 500, &empty_id) != ERR_SUCCESS)
		return false;
	return true;
}

int fp_get_empty_id() {
    int empty_id;

	if (m_comm->Run_GetEmptyID(1, 500, &empty_id) != ERR_SUCCESS)
		return -1;
	return empty_id;
}

int fp_generate(int ram_buffer_id) {
    return m_comm->Run_Generate(ram_buffer_id);
}

int fp_merge(int ram_buffer_id) {
    return m_comm->Run_Merge(0, ram_buffer_id);
}

int fp_search(int *id) {
    int learnResult;
    // 运行搜索函数，根据返回值判断是否搜索成功
    int ret = m_comm.Run_Search(0, 1, 200, id, &learnResult);
    return (ret == ERR_SUCCESS);
}