#include <stdio.h>
#include <thread>
#include <chrono>

#include "device.h"
#include "define.h"
#include "communication.h"
#include "fingerprint.h"

// 定义检查周期为 500 毫秒
#define PERIOD_CHECK    500        // 500ms
// 定义任务周期为 1000 毫秒
#define PERIOD_TASK     1000       // 1000ms

/**
 * @brief 指纹工作类的构造函数
 * @param fp_devname 指纹设备名称
 */
FingerPrint_Worker::FingerPrint_Worker(const std::string& fp_devname)
    : m_comm(fp_devname)
{
    // 初始化设备离线状态
    m_online = false;
    // 初始化可用 ID 为 0
    m_avail_id = 0;
    // 初始化新模板标志为 false
    m_new_template = false;
    // 初始化检查定时器指针为 nullptr
    m_check_timer = nullptr;
    // 初始化任务定时器指针为 nullptr
    m_task_timer = nullptr;
}

/**
 * @brief 指纹工作类的析构函数，停止相关操作
 */
FingerPrint_Worker::~FingerPrint_Worker()
{
    // 停止指纹设备工作
    stop();
}

/**
 * @brief 启动指纹设备工作
 */
void FingerPrint_Worker::start()
{
    // 初始化设备并检查是否在线
    m_online = init();
    if (!m_online) {
        // 初始化失败，输出错误信息
        printf("[FINGERPRINT] Initialize failed\n");
        return;
    } else {
        // 初始化成功，更新可用 ID
        update_avail_id();
    }

    // 创建检查定时器
    m_check_timer = new Timer();
    // 创建任务定时器
    m_task_timer = new Timer();
}

/**
 * @brief 停止指纹设备工作
 */
void FingerPrint_Worker::stop()
{
    // 标记设备离线
    m_online = false;
    
    // 如果检查定时器存在，停止并释放资源
    if (m_check_timer) {
        m_check_timer->stop();
        delete m_check_timer;
        m_check_timer = nullptr;
    }
    
    // 如果任务定时器存在，停止并释放资源
    if (m_task_timer) {
        m_task_timer->stop();
        delete m_task_timer;
        m_task_timer = nullptr;
    }
}

/**
 * @brief 开始指纹检测
 */
void FingerPrint_Worker::fp_detect_start()
{
    // 如果设备离线，直接返回
    if (!m_online)
        return;

    // 如果任务定时器存在，停止任务定时器
    if (m_task_timer) {
        m_task_timer->stop();
    }
    
    // 初始化检查倒计时
    m_check_countdown = 1;
    // 如果检查定时器存在，启动检查定时器并设置回调函数
    if (m_check_timer) {
        m_check_timer->start(PERIOD_CHECK, [this]() { fp_check(); });
    }
}

/**
 * @brief 停止指纹检测
 */
void FingerPrint_Worker::fp_detect_stop()
{
    // 如果设备离线，直接返回
    if (!m_online)
        return;

    // 标记新模板标志为 false
    m_new_template = false;
    // 如果检查定时器存在，停止检查定时器
    if (m_check_timer) {
        m_check_timer->stop();
    }
}

/**
 * @brief 开始指纹录入
 */
void FingerPrint_Worker::fp_enroll_start()
{
    // 如果设备离线，直接返回
    if (!m_online)
        return;

    // 输出指纹录入开始信息
    printf("[FINGERPRINT] Enroll fingerprint\n");

    // 如果检查定时器存在，停止检查定时器
    if (m_check_timer) {
        m_check_timer->stop();
    }
    
    // 初始化当前处理步骤为移除手指
    m_task.curr_proc = FP_REMOVE_FINGER;
    // 初始化录入步骤为 0
    m_task.enroll_step = 0;
    // 标记新模板标志为 false
    m_new_template = false;
    
    // 如果任务定时器存在，启动任务定时器并设置回调函数
    if (m_task_timer) {
        m_task_timer->start(PERIOD_TASK, [this]() { fp_task(); });
    }
}

/**
 * @brief 保存指纹模板
 * @param finger_id 要保存的指纹 ID
 */
void FingerPrint_Worker::fp_save(uint32_t finger_id)
{
    // 如果设备离线，直接返回
    if (!m_online)
        return;

    // 如果任务定时器存在，停止任务定时器
    if (m_task_timer) {
        m_task_timer->stop();
    }

    // 存储指纹模板
    store(finger_id);
    // 标记新模板标志为 false
    m_new_template = false;
}

/**
 * @brief 删除指定 ID 的指纹模板
 * @param finger_id 要删除的指纹 ID
 */
void FingerPrint_Worker::fp_delete(uint32_t finger_id)
{
    // 如果设备离线，直接返回
    if (!m_online)
        return;

    // 删除指定 ID 的指纹模板
    remove(finger_id);
}

/**
 * @brief 处理指纹录入任务
 */
void FingerPrint_Worker::fp_task()
{
    switch (m_task.curr_proc) {
    case FP_REMOVE_FINGER:
        // 检测手指是否存在
        if (detect_finger()) {
            // 如果存在手指，调用消息回调函数提示移除手指
            if (m_messageCallback) {
                m_messageCallback((uint32_t)FP_REMOVE_FINGER, m_task.enroll_step);
            }
            break;
        }
        // 手指已移除，切换到放置手指步骤
        m_task.curr_proc = FP_PLACE_FINGER;
        /* FALLTHROUGH */ 
    case FP_PLACE_FINGER:
        // 检测手指是否不存在
        if (!detect_finger()) {
            // 如果手指不存在，调用消息回调函数提示放置手指
            if (m_messageCallback) {
                m_messageCallback((uint32_t)FP_PLACE_FINGER, m_task.enroll_step);
            }
            break;
        }
        // 捕获指纹图像，如果失败则跳出
        if (capture() != ERR_SUCCESS)
            break;
        // 生成指纹模板
        generate(m_task.enroll_step);
        // 录入步骤加 1
        m_task.enroll_step++;
        // 如果录入步骤不等于 3，切换到移除手指步骤
        if (m_task.enroll_step != 3) {
            m_task.curr_proc = FP_REMOVE_FINGER;
            break;
        }
        // 录入步骤达到 3，切换到合并模板步骤
        m_task.curr_proc = FP_MERGE_TEMPLATE;
        break;
    case FP_CAPTURE_FINGER:
        break;
    case FP_GENERATE_TEMPLATE:
        break;
    case FP_MERGE_TEMPLATE:
        // 合并指纹模板，如果成功
        if (merge(3) == ERR_SUCCESS) {
            // 检查是否有重复模板
            if (!duplicated()) {
                // 没有重复模板，标记新模板标志为 true
                m_new_template = true;
                // 调用消息回调函数提示合并成功
                if (m_messageCallback) {
                    m_messageCallback((uint32_t)FP_MERGE_TEMPLATE, m_task.enroll_step);
                }
            }
            else {
                // 有重复模板，调用消息回调函数提示重复
                if (m_messageCallback) {
                    m_messageCallback((uint32_t)FP_DUPLICATE_TEMPLATE, 0);
                }
            }
        }
        // 任务完成，切换到无任务状态
        m_task.curr_proc = FP_NONE;
        // 如果任务定时器存在，停止任务定时器
        if (m_task_timer) {
            m_task_timer->stop();
        }
        break;
    case FP_SAVE_TEMPLATE:
        break;
    default:
        break;
    }
}

/**
 * @brief 检查指纹是否匹配
 */
void FingerPrint_Worker::fp_check()
{
    int id;

    // 检查倒计时减 1
    if (m_check_countdown)
        m_check_countdown--;
    // 如果倒计时未结束，直接返回
    if (m_check_countdown)
        return;
    // 检测手指是否存在
    if (detect_finger()) {
        // 捕获指纹图像
        capture();
        // 生成指纹模板
        generate(0);
        // 搜索匹配的指纹 ID
        if (search(&id)) {
            // 如果找到匹配的 ID，调用检测回调函数
            if (m_detectionCallback) {
                m_detectionCallback(id);
            }
            // 重置检查倒计时
            m_check_countdown = 10;
        }
    }
}

/**
 * @brief 初始化指纹设备连接
 * @return 连接成功返回 true，失败返回 false
 */
bool FingerPrint_Worker::init()
{
    // 运行初始化连接函数，如果连接失败则返回 false
    if (m_comm.Run_InitConnection() != CONNECTION_SUCCESS)
        return false;
    return true;
}

// [其余底层方法保持不变]
/**
 * @brief 检查指纹模板是否重复
 * @return 重复返回 true，不重复返回 false
 */
bool FingerPrint_Worker::duplicated()
{
    // 简单实现，实际业务逻辑需要您填充
    return false;
}

/**
 * @brief 删除指定 ID 的指纹模板
 * @param id 要删除的指纹 ID
 * @return 删除操作的返回值
 */
int FingerPrint_Worker::remove(int id)
{
    return m_comm.Run_DelChar(id, id);
}

/**
 * @brief 存储指纹模板到指定 ID
 * @param id 要存储的指纹 ID
 * @return 存储操作的返回值
 */
int FingerPrint_Worker::store(int id)
{
    int dupId;
    return m_comm.Run_StoreChar(id, 0, &dupId);
}

/**
 * @brief 生成指纹模板
 * @param step 生成步骤
 * @return 生成操作的返回值
 */
int FingerPrint_Worker::generate(int step)
{
    return m_comm.Run_Generate(step);
}

/**
 * @brief 更新可用的指纹 ID
 */
void FingerPrint_Worker::update_avail_id()
{
    int emptyId;
    // 获取可用的指纹 ID，如果成功则更新 m_avail_id
    if (m_comm.Run_GetEmptyID(1, 200, &emptyId) == ERR_SUCCESS) {
        m_avail_id = emptyId;
    } else {
        // 获取失败，将 m_avail_id 设置为 0
        m_avail_id = 0;
    }
}

/**
 * @brief 捕获指纹图像
 * @return 捕获操作的返回值
 */
int FingerPrint_Worker::capture()
{
    return m_comm.Run_GetImage();
}

/**
 * @brief 检测手指是否存在
 * @return 存在返回 true，不存在返回 false
 */
bool FingerPrint_Worker::detect_finger()
{
    int result;
    // 运行手指检测函数，根据返回值判断手指是否存在
    int ret = m_comm.Run_FingerDetect(&result);
    return (ret == ERR_SUCCESS && result == 1);
}

/**
 * @brief 搜索匹配的指纹 ID
 * @param id 用于存储匹配到的指纹 ID
 * @return 搜索成功返回 true，失败返回 false
 */
bool FingerPrint_Worker::search(int* id)
{
    int learnResult;
    // 运行搜索函数，根据返回值判断是否搜索成功
    int ret = m_comm.Run_Search(0, 1, 200, id, &learnResult);
    return (ret == ERR_SUCCESS);
}

/**
 * @brief 合并指纹模板
 * @param step 合并步骤
 * @return 合并操作的返回值
 */
int FingerPrint_Worker::merge(int step)
{
    return m_comm.Run_Merge(0, step);
}