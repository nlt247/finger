#ifndef __FINGER_PRINT__
#define __FINGER_PRINT__

#include <vector>
#include <functional>
#include "thread.h"
#include "communication.h"

class FingerPrint_Worker {
public:
    enum FP_ENROLL_PROCEDURE {
        FP_NONE,
        FP_PLACE_FINGER,
        FP_REMOVE_FINGER,
        FP_CAPTURE_FINGER,
        FP_GENERATE_TEMPLATE,
        FP_MERGE_TEMPLATE,
        FP_SAVE_TEMPLATE,
        FP_DUPLICATE_TEMPLATE
    };

    // 回调函数类型
    typedef std::function<void(int id)> DetectionCallback;
    typedef std::function<void(uint32_t proc, uint32_t enroll_step)> MessageCallback;

    FingerPrint_Worker(const std::string& fp_devname);
    ~FingerPrint_Worker();

    // 设置回调
    void setDetectionCallback(DetectionCallback callback) { m_detectionCallback = callback; }
    void setMessageCallback(MessageCallback callback) { m_messageCallback = callback; }

    // 操作方法
    void start();
    void stop();
    void fp_detect_start();
    void fp_detect_stop();
    void fp_enroll_start();
    void fp_save(uint32_t finger_id);
    void fp_delete(uint32_t finger_id);

    // 状态查询
    bool online() const { return m_online; }
    int avail_id() const { return m_avail_id; }
    bool new_template_avail() const { return m_new_template; }

private:
    // 私有任务处理方法
    void fp_task();
    void fp_check();

    // 指纹操作方法
    int capture();
    int generate(int step);
    int merge(int step);
    bool search(int *id);
    bool duplicated();
    bool detect_finger();
    bool id_avail(int id);
    int store(int id);
    int remove(int id);
    int remove_all();

    // 初始化和状态更新
    bool init();
    void update_avail_id();

    // 内部任务结构
    struct Task {
        FP_ENROLL_PROCEDURE curr_proc;
        uint32_t enroll_step;
        uint32_t waiting_time;
    };

    // 成员变量
    bool m_online;
    CCommunication m_comm;
    uint32_t m_avail_id;
    bool m_new_template;
    
    Timer* m_check_timer;
    uint32_t m_check_countdown;
    Timer* m_task_timer;
    Task m_task;
    
    DetectionCallback m_detectionCallback;
    MessageCallback m_messageCallback;
};

#endif