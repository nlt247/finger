#include <QCoreApplication>
#include <QTimer>
#include <QDebug>

#include "setting/JSON_settings.h"
#include "coordinator/coordinator.h"

#include "define.h"
#include "communication.h"
#include "fingerprint.h"

#define PERIOD_CHECK		500				// 500ms
#define PERIOD_TASK			1000			// 200ms

FingerPrint_Worker::FingerPrint_Worker(QString fp_devname, QObject *parent)
    : QObject(parent), m_comm(fp_devname)
{
	m_online = false;
	m_avail_id = 0;
	m_new_template = false;
}

FingerPrint_Worker::~FingerPrint_Worker()
{
}

void FingerPrint_Worker::start()
{
	m_online = init();
	if (!m_online) {
        qInfo() << "[FINGERPRINT]Initialize failed";
        QTimer::singleShot(10000, this, &FingerPrint_Worker::start);
        return;
    }
    else {
		update_avail_id();
    }

	m_check_timer = new QTimer(this);
    connect(m_check_timer, &QTimer::timeout, this, &FingerPrint_Worker::fp_check);
	m_task_timer = new QTimer(this);
    connect(m_task_timer, &QTimer::timeout, this, &FingerPrint_Worker::fp_task);
}

void FingerPrint_Worker::stop()
{
	m_online = false;
	delete m_check_timer;
}

void FingerPrint_Worker::fp_detect_start()
{
	if (!m_online)
		return;

	m_task_timer->stop();
	m_check_timer->start(PERIOD_CHECK);
	m_check_countdown = 1;
}

void FingerPrint_Worker::fp_detect_stop()
{
	if (!m_online)
		return;

	m_new_template = false;
	m_check_timer->stop();
}

void FingerPrint_Worker::fp_enroll_start()
{
	if (!m_online)
		return;

    qInfo() << "[FINGERPRINT]Enroll fingerprint";

	m_check_timer->stop();
	m_task.curr_proc = FP_REMOVE_FINGER;
	m_task.enroll_step = 0;
	m_new_template = false;
	m_task_timer->start(PERIOD_TASK);
}

void FingerPrint_Worker::fp_save(uint32_t finger_id)
{
	if (!m_online)
		return;

	m_task_timer->stop();

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
    		emit fp_message((uint32_t) FP_REMOVE_FINGER, m_task.enroll_step);
    		break;
    	}
   		m_task.curr_proc = FP_PLACE_FINGER;
    case FP_PLACE_FINGER:
    	if (!detect_finger()) {
    		emit fp_message((uint32_t) FP_PLACE_FINGER, m_task.enroll_step);
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
	    		emit fp_message((uint32_t) FP_MERGE_TEMPLATE, m_task.enroll_step);
    		}
    		else
	    		emit fp_message((uint32_t) FP_DUPLICATE_TEMPLATE, 0);
    	}
    	m_task.curr_proc = FP_NONE;
		m_task_timer->stop();
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
			emit fp_dected(id);
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

int FingerPrint_Worker::capture()
{
	return m_comm.Run_GetImage();
}

int FingerPrint_Worker::generate(int step)
{
	return m_comm.Run_Generate(step);
}

int FingerPrint_Worker::merge(int step)
{
	return m_comm.Run_Merge(0, step);
}

bool FingerPrint_Worker::search(int *id)
{
	int result;

	return (m_comm.Run_Search(0, 1, 500, id, &result) == ERR_SUCCESS);
}

bool FingerPrint_Worker::duplicated()
{
	int result;
	int id;

	if (m_comm.Run_Search(0, 1, 500, &id, &result) == ERR_SUCCESS)
		return true;
	return false;
}

bool FingerPrint_Worker::detect_finger()
{
	int ret_code;

	if (m_comm.Run_FingerDetect(&ret_code) != ERR_SUCCESS)
		return false;

	return ret_code;
}

bool FingerPrint_Worker::id_avail(int id)
{
	int empty_id;

	if (m_comm.Run_GetEmptyID(id, id, &empty_id) != ERR_SUCCESS)
		return false;
	return true;
}

int FingerPrint_Worker::store(int id)
{
	int dup_id, ret_code;

	remove(id);
	ret_code = m_comm.Run_StoreChar(id, 0, &dup_id);
	update_avail_id();

	return ret_code;
}

int FingerPrint_Worker::remove(int id)
{
	int ret_code;

	ret_code = m_comm.Run_DelChar(id, id);
	update_avail_id();

	return ret_code;
}

int FingerPrint_Worker::remove_all()
{
	int ret_code;

	ret_code = m_comm.Run_DelChar(1, 500);
	update_avail_id();

	return ret_code;
}

void FingerPrint_Worker::update_avail_id()
{
	int empty_id;

	if (m_comm.Run_GetEmptyID(1, 500, &empty_id) != ERR_SUCCESS)
		m_avail_id = 0;
	else
		m_avail_id = empty_id;
}


FingerPrintController::FingerPrintController(QObject *parent)
	: m_worker_thread()
{
    QSettings settings(JSONSettings::GetSettingFilePath(), JSONSettings::GetFormat(), QCoreApplication::instance());
    QString fp_devname = settings.value(JSON_FP_DEVICE).toString();

	m_coordinator = qobject_cast<Coordinator *>(parent);

    m_worker = new FingerPrint_Worker(fp_devname);
	m_worker->moveToThread(&m_worker_thread);

	connect(&m_worker_thread, &QThread::finished, m_worker, &QObject::deleteLater);
    connect(this, &FingerPrintController::start, m_worker, &FingerPrint_Worker::start);
    connect(this, &FingerPrintController::stop, m_worker, &FingerPrint_Worker::stop);

    m_worker_thread.setObjectName("FingerPrint Thread");
	m_worker_thread.start();
}

FingerPrintController::~FingerPrintController()
{
	m_worker_thread.quit();
	m_worker_thread.wait();
}
