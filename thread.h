#ifndef THREAD_H
#define THREAD_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional>
#include <atomic>
#include <queue>
#include <chrono>

class Thread {
public:
    Thread() : m_running(false), m_thread(nullptr) {}
    
    ~Thread() {
        stop();
    }
    
    void start(std::function<void()> func) {
        if (m_running) return;
        
        m_running = true;
        m_thread = new std::thread([this, func]() {
            while (m_running) {
                func();
            }
        });
    }
    
    void stop() {
        if (!m_running) return;
        
        m_running = false;
        if (m_thread && m_thread->joinable()) {
            m_thread->join();
            delete m_thread;
            m_thread = nullptr;
        }
    }
    
private:
    std::atomic<bool> m_running;
    std::thread* m_thread;
};

// 定时器类
class Timer {
public:
    Timer() : m_running(false), m_thread(nullptr) {}
    
    ~Timer() {
        stop();
    }
    
    void start(int interval_ms, std::function<void()> callback) {
        if (m_running) return;
        
        m_running = true;
        m_thread = new std::thread([this, interval_ms, callback]() {
            while (m_running) {
                std::this_thread::sleep_for(std::chrono::milliseconds(interval_ms));
                if (m_running) {
                    callback();
                }
            }
        });
    }
    
    void stop() {
        if (!m_running) return;
        
        m_running = false;
        if (m_thread && m_thread->joinable()) {
            m_thread->join();
            delete m_thread;
            m_thread = nullptr;
        }
    }
    
private:
    std::atomic<bool> m_running;
    std::thread* m_thread;
};

// 消息队列
template<typename T>
class MessageQueue {
public:
    void push(const T& item) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_queue.push(item);
        m_cond.notify_one();
    }
    
    bool pop(T& item, int timeout_ms = -1) {
        std::unique_lock<std::mutex> lock(m_mutex);
        
        if (timeout_ms < 0) {
            // 无超时等待
            m_cond.wait(lock, [this] { return !m_queue.empty(); });
            item = m_queue.front();
            m_queue.pop();
            return true;
        } else if (timeout_ms == 0) {
            // 非阻塞模式
            if (m_queue.empty()) {
                return false;
            }
            item = m_queue.front();
            m_queue.pop();
            return true;
        } else {
            // 带超时等待
            bool success = m_cond.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                                         [this] { return !m_queue.empty(); });
            if (success) {
                item = m_queue.front();
                m_queue.pop();
                return true;
            }
            return false;
        }
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_queue.empty();
    }
    
private:
    std::queue<T> m_queue;
    mutable std::mutex m_mutex;
    std::condition_variable m_cond;
};

#endif // THREAD_H