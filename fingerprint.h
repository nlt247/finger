#ifndef __FINGER_PRINT__
#define __FINGER_PRINT__

#include <QObject>
#include <QThread>

#include "common.h"
#include "communication.h"

class Coordinator;
class QTimer;

class FingerPrint_Worker : public QObject
{

	Q_OBJECT

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
	typedef enum FP_ENROLL_PROCEDURE FP_ENROLL_PROCEDURE;

	FingerPrint_Worker(QString fp_devname, QObject *parent = nullptr);
	~FingerPrint_Worker();

signals:

	void fp_dected(int id);
    void fp_message(uint32_t proc, uint32_t enroll_step);

public slots:

	void start();
	void stop();

	void fp_detect_start();
	void fp_detect_stop();
	void fp_enroll_start();
	void fp_save(uint32_t finger_id);
	void fp_delete(uint32_t finger_id);

private slots:

	void fp_task();
	void fp_check();

public:

	bool online() { return m_online; };
	int avail_id() { return m_avail_id; };
	bool new_template_avail() { return m_new_template; };

private:

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

private:

	struct Task {
        FP_ENROLL_PROCEDURE curr_proc;
		uint32_t enroll_step;
		uint32_t waiting_time;
	};
	typedef struct Task Task;

private:

	bool init();
	void update_avail_id();

	bool m_online;
	CCommunication m_comm;
	uint32_t m_avail_id;
	bool m_new_template;

	QTimer *m_check_timer;
	uint32_t m_check_countdown;
	QTimer *m_task_timer;
	Task m_task;
};

class FingerPrintController : public QObject
{
	Q_OBJECT

public:

    FingerPrintController(QObject *parent = nullptr);
    ~FingerPrintController();

    FingerPrint_Worker *getWorker() {
        return m_worker;
    }

signals:

	void start();
	void stop();

private:

	QThread m_worker_thread;
	FingerPrint_Worker *m_worker;

    Coordinator *m_coordinator;
};

#endif
