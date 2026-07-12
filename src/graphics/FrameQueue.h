#pragma once
#include <QMutex>
#include <QQueue>
#include <QWaitCondition>
#include <QByteArray>

struct Frame {
    QByteArray pixels;
    int width;
    int height;
};

class FrameQueue {
public:
    FrameQueue() = default;
    ~FrameQueue() = default;

    void push(const Frame& frame) {
        QMutexLocker locker(&m_mutex);
        // Triple buffer limit: drop old frames to prevent latency build-up
        while (m_queue.size() >= 3) {
            m_queue.dequeue();
        }
        m_queue.enqueue(frame);
        m_cond.wakeOne();
    }

    bool pop(Frame& frame, int timeoutMs = 100) {
        QMutexLocker locker(&m_mutex);
        if (m_queue.isEmpty()) {
            if (!m_cond.wait(&m_mutex, timeoutMs)) {
                return false;
            }
        }
        if (m_queue.isEmpty()) {
            return false;
        }
        frame = m_queue.dequeue();
        return true;
    }

    void clear() {
        QMutexLocker locker(&m_mutex);
        m_queue.clear();
    }

    bool isEmpty() {
        QMutexLocker locker(&m_mutex);
        return m_queue.isEmpty();
    }

private:
    QMutex m_mutex;
    QWaitCondition m_cond;
    QQueue<Frame> m_queue;
};
