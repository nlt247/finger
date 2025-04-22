#include <stdio.h>
#include <thread>
#include <chrono>

#include "device.h"
#include "define.h"
#include "communication.h"
#include "fingerprint.h"

#define PERIOD_CHECK    500        // 500ms
#define PERIOD_TASK     1000       // 1000ms

FingerPrint_Worker::FingerPrint_Worker(const std::string& fp_devname)
    : m_comm(fp_devname)
{
    m_online = false;
    m_avail_id = 0;
    m_new_template = false;
    m_check_timer = nullptr;
    m_task_timer = nullptr;
}

FingerPrint_Worker::~FingerPrint_Worker()
{
    stop();
}

void FingerPrint_Worker::start()
{
    m_online = init();
    if (!m_online) {
        printf("[FINGERPRINT] Initialize failed\n");
        return;
    } else {
        update_avail_id();
    }

    m_check_timer = new Timer();
    m_task_timer = new Timer();
}

void FingerPrint_Worker::stop()
{
    m_online = false;
    
    if (m_check_timer) {
        m_check_timer->stop();
        delete m_check_timer;
        m_check_timer = nullptr;
    }
    
    if (m_task_timer) {
        m_task_timer->stop();
        delete m_task_timer;
        m_task_timer = nullptr;
    }
}

void FingerPrint_Worker::fp_detect_start()
{
    if (!m_online)
        return;

    if (m_task_timer) {
        m_task_timer->stop();
    }
    
    m_check_countdown = 1;
    if (m_check_timer) {
        m_check_timer->start(PERIOD_CHECK, [this]() { fp_check(); });
    }
}

void FingerPrint_Worker::fp_detect_stop()
{
    if (!m_online)
        return;

    m_new_template = false;
    if (m_check_timer) {
        m_check_timer->stop();
    }
}

void FingerPrint_Worker::fp_enroll_start()
{
    if (!m_online)
        return;

    printf("[FINGERPRINT] Enroll fingerprint\n");

    if (m_check_timer) {
        m_check_timer->stop();
    }
    
    m_task.curr_proc = FP_REMOVE_FINGER;
    m_task.enroll_step = 0;
    m_new_template = false;
    
    if (m_task_timer) {
        m_task_timer->start(PERIOD_TASK, [this]() { fp_task(); });
    }
}

void FingerPrint_Worker::fp_save(uint32_t finger_id)
{
    if (!m_online)
        return;

    if (m_task_timer) {
        m_task_timer->stop();
    }

    store(finger_id);
    m_new_template = false;
}

void FingerPrint_Worker::fp_delete(uint32_t finger_id)
{
    if (!m_online)
        return;

    remove(finger_id);
}

void FingerPrint_Worker::fp_task()
{
    switch (m_task.curr_proc) {
    case FP_REMOVE_FINGER:
        if (detect_finger()) {
            if (m_messageCallback) {
                m_messageCallback((uint32_t)FP_REMOVE_FINGER, m_task.enroll_step);
            }
            break;
        }
        m_task.curr_proc = FP_PLACE_FINGER;
        /* FALLTHROUGH */ 
    case FP_PLACE_FINGER:
        if (!detect_finger()) {
            if (m_messageCallback) {
                m_messageCallback((uint32_t)FP_PLACE_FINGER, m_task.enroll_step);
            }
            break;
        }
        if (capture() != ERR_SUCCESS)
            break;
        generate(m_task.enroll_step);
        m_task.enroll_step++;
        if (m_task.enroll_step != 3) {
            m_task.curr_proc = FP_REMOVE_FINGER;
            break;
        }
        m_task.curr_proc = FP_MERGE_TEMPLATE;
        break;
    case FP_CAPTURE_FINGER:
        break;
    case FP_GENERATE_TEMPLATE:
        break;
    case FP_MERGE_TEMPLATE:
        if (merge(3) == ERR_SUCCESS) {
            if (!duplicated()) {
                m_new_template = true;
                if (m_messageCallback) {
                    m_messageCallback((uint32_t)FP_MERGE_TEMPLATE, m_task.enroll_step);
                }
            }
            else {
                if (m_messageCallback) {
                    m_messageCallback((uint32_t)FP_DUPLICATE_TEMPLATE, 0);
                }
            }
        }
        m_task.curr_proc = FP_NONE;
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

void FingerPrint_Worker::fp_check()
{
    int id;

    if (m_check_countdown)
        m_check_countdown--;
    if (m_check_countdown)
        return;
    if (detect_finger()) {
        capture();
        generate(0);
        if (search(&id)) {
            if (m_detectionCallback) {
                m_detectionCallback(id);
            }
            m_check_countdown = 10;
        }
    }
}

bool FingerPrint_Worker::init()
{
    if (m_comm.Run_InitConnection() != CONNECTION_SUCCESS)
        return false;
    return true;
}

// [其余底层方法保持不变]
void CCommunication::CloseConnection()
{
    USBSCSIDeInit();
}

bool FingerPrint_Worker::duplicated()
{
    // 简单实现，实际业务逻辑需要您填充
    return false;
}

int FingerPrint_Worker::remove(int id)
{
    return m_comm.Run_DelChar(id, id);
}

int FingerPrint_Worker::store(int id)
{
    int dupId;
    return m_comm.Run_StoreChar(id, 0, &dupId);
}

int FingerPrint_Worker::generate(int step)
{
    return m_comm.Run_Generate(step);
}

void FingerPrint_Worker::update_avail_id()
{
    int emptyId;
    if (m_comm.Run_GetEmptyID(1, 200, &emptyId) == ERR_SUCCESS) {
        m_avail_id = emptyId;
    } else {
        m_avail_id = 0;
    }
}

int FingerPrint_Worker::capture()
{
    return m_comm.Run_GetImage();
}

bool FingerPrint_Worker::detect_finger()
{
    int result;
    int ret = m_comm.Run_FingerDetect(&result);
    return (ret == ERR_SUCCESS && result == 1);
}

bool FingerPrint_Worker::search(int* id)
{
    int learnResult;
    int ret = m_comm.Run_Search(0, 1, 200, id, &learnResult);
    return (ret == ERR_SUCCESS);
}

int FingerPrint_Worker::merge(int step)
{
    return m_comm.Run_Merge(0, step);
}