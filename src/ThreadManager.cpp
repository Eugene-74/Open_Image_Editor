#pragma unroll

#include "ThreadManager.hpp"

#include <QDebug>
#include <QThreadPool>
#include <queue>

#include "Const.hpp"

/**
 * @brief Worker to lunch a thread
 * @param job function to run
 */
Worker::Worker(std::function<void()> job, std::function<void()> process)
    : job(std::move(job)), process(std::move(process)) {}

/**
 * @brief Close all threads and clear thread queue
 */
ThreadManager::~ThreadManager() {
    removeAllThreads();
    QThreadPool::globalInstance()->waitForDone();
}

/**
 * @brief Run the thread
 */
void Worker::run() {
    try {
        if (job) job();
        if (process) process();
    } catch (const std::exception& e) {
        qCritical() << "Exception in Worker::run: " << e.what();
    } catch (...) {
        qCritical() << "Unknown exception in Worker::run";
    }
}

/**
 * @brief Create a thread manager
 * @param parent Parent of the manager
 * @param maxThreads maxThreads of the manager
 */
ThreadManager::ThreadManager(QObject* parent, int maxThreads)
    : QObject(parent), maxThreads(maxThreads) {
    QThreadPool::globalInstance()->setMaxThreadCount(maxThreads);
}

/**
 * @brief Add a heavy thread to the thread pool queue, so it will only be loaded if ther is a free thread and no thread in the queue
 * @param job function for the thread
 */
void ThreadManager::addHeavyThread(std::function<void()> job) {
    if (QThreadPool::globalInstance()->activeThreadCount() <= maxThreads - FREE_THREAD) {
        startJob(std::move(job));
    } else {
        {
            std::lock_guard<std::mutex> lock(heavyTaskQueueMutex);
            heavyTaskQueue.push_back(std::move(job));
        }
        if (heavyTaskQueue.size() > 100) {
            qWarning() << "Heavy thread queue" << heavyTaskQueue.size();
        }
    }
}

/**
 * @brief Add a thread to the thread pool queue
 * @param job function for the thread
 */
void ThreadManager::addThread(std::function<void()> job) {
    if (QThreadPool::globalInstance()->activeThreadCount() < maxThreads) {
        startJob(std::move(job));

        if (taskQueue.size() > 100) {
            qWarning() << "Thread queue" << taskQueue.size();
        }
    } else {
        if (taskQueue.size() < 1000) {
            {
                std::lock_guard<std::mutex> lock(taskQueueMutex);
                taskQueue.push_back(std::move(job));
            }
            qInfo() << "Job added to queue. Queue size: " << taskQueue.size();
        } else {
            qCritical() << "Thread queue is full. Job discarded.";
        }
    }
}

/**
 * @brief Add a thread to the thread pool queue but in the front of the queue
 * @param job function for the thread
 */
void ThreadManager::addThreadToFront(std::function<void()> job) {
    if (QThreadPool::globalInstance()->activeThreadCount() <= maxThreads - FREE_THREAD) {
        startJob(std::move(job));
    } else {
        {
            std::lock_guard<std::mutex> lock(taskQueueMutex);
            heavyTaskQueue.push_front(std::move(job));
        }
        if (heavyTaskQueue.size() > 100) {
            qWarning() << "Heavy thread queue" << heavyTaskQueue.size();
        }
    }
}

/**
 * @brief Add a thread to the thread pool queue but in the front of the heavy task queue
 * @param job function for the thread
 */
void ThreadManager::addHeavyThreadToFront(std::function<void()> job) {
    if (QThreadPool::globalInstance()->activeThreadCount() < maxThreads) {
        startJob(std::move(job));

        if (taskQueue.size() > 100) {
            qWarning() << "Thread queue" << taskQueue.size();
        }
    } else {
        if (taskQueue.size() < 1000) {
            {
                std::lock_guard<std::mutex> lock(taskQueueMutex);
                taskQueue.push_front(std::move(job));
            }
            qInfo() << "Job added to queue. Queue size: " << taskQueue.size();
        } else {
            qCritical() << "Thread queue is full. Job discarded.";
        }
    }
}

/**
 * @brief Strop all active threads
 */
void ThreadManager::removeAllThreads() {
    while (!taskQueue.empty()) {
        {
            std::lock_guard<std::mutex> lock(taskQueueMutex);
            taskQueue.clear();
        }
    }
    while (!heavyTaskQueue.empty()) {
        {
            std::lock_guard<std::mutex> lock(heavyTaskQueueMutex);
            heavyTaskQueue.clear();
        }
    }
    QThreadPool::globalInstance()->clear();
    qInfo() << "All threads stopped";
}

/**
 * @brief Give you the number of active thread in the thread pool
 * @return number of active thread
 */
int ThreadManager::getThreadCount() const {
    return QThreadPool::globalInstance()->activeThreadCount();
}

/**
 * @brief Manage the thread queue when it's not empty
 */
void ThreadManager::processQueue() {
    if (!taskQueue.empty() && QThreadPool::globalInstance()->activeThreadCount() <= maxThreads) {
        std::function<void()> job = std::move(taskQueue.front());
        {
            std::lock_guard<std::mutex> lock(taskQueueMutex);
            taskQueue.pop_front();
        }
        if (job) {
            startJob(std::move(job));
        }

    } else if (!heavyTaskQueue.empty() && (QThreadPool::globalInstance()->activeThreadCount() <= maxThreads - FREE_THREAD)) {
        std::function<void()> job = std::move(heavyTaskQueue.front());
        {
            std::lock_guard<std::mutex> lock(heavyTaskQueueMutex);
            heavyTaskQueue.pop_front();
        }
        if (job) {
            startJob(std::move(job));
        }
    }
}

/**
 * @brief Start a job in the thread pool
 * @param job function for the thread
 */
void ThreadManager::startJob(std::function<void()> job) {
    try {
        int activeThreads = QThreadPool::globalInstance()->activeThreadCount();
        if (activeThreads >= maxThreads) {
            qWarning() << "Cannot start job. Active threads: " << activeThreads << ", Max threads: " << maxThreads;

            {
                std::lock_guard<std::mutex> lock(taskQueueMutex);
                taskQueue.push_back(std::move(job));
            }

            return;
        }

        Worker* worker = new Worker(std::move(job), [this]() { processQueue(); });
        if (!QThreadPool::globalInstance()->tryStart(worker)) {
            qCritical() << "Could not start thread";
            qInfo() << "Could not start thread. Active threads: " << getThreadCount();
            // taskQueue.push_front(std::move(worker->job));  // Re-add the job to the front of the queue
            delete worker;
        } else {
            qInfo() << "Thread started successfully. Active threads: " << getThreadCount();
        }
    } catch (const std::exception& e) {
        qCritical() << "Exception in startJob: " << e.what();
    } catch (...) {
        qCritical() << "Unknown exception in startJob";
    }
}