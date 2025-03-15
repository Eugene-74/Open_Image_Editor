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
    if (job) job();
    if (process) process();
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
        Worker* worker = new Worker(std::move(job), [this]() { processQueue(); });
        QThreadPool::globalInstance()->start(worker);
    } else {
        heavyTaskQueue.push_back(std::move(job));
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
        taskQueue.push_back(std::move(job));
        qInfo() << "Job added to queue. Queue size: " << taskQueue.size();
    }
}

/**
 * @brief Add a thread to the thread pool queue but in the front of the queue
 * @param job function for the thread
 */
void ThreadManager::addThreadToFront(std::function<void()> job) {
    taskQueue.push_front(std::move(job));
    qInfo() << "Job added to front of queue. Queue size: " << taskQueue.size();
}

/**
 * @brief Add a thread to the thread pool queue but in the front of the heavy task queue
 * @param job function for the thread
 */
void ThreadManager::addHeavyThreadToFront(std::function<void()> job) {
    heavyTaskQueue.push_front(std::move(job));
    qInfo() << "Job added to front of queue. Queue size: " << heavyTaskQueue.size();
}

/**
 * @brief Strop all active threads
 */
void ThreadManager::removeAllThreads() {
    QThreadPool::globalInstance()->clear();
    taskQueue.clear();
    heavyTaskQueue.clear();
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
        taskQueue.pop_front();
        startJob(std::move(job));

    } else if (!heavyTaskQueue.empty() && QThreadPool::globalInstance()->activeThreadCount() <= maxThreads - FREE_THREAD) {
        // qDebug() << "Process heavy task queue : " << heavyTaskQueue.size();
        std::function<void()> job = std::move(heavyTaskQueue.front());
        heavyTaskQueue.pop_front();
        startJob(std::move(job));
    }
}

void ThreadManager::startJob(std::function<void()> job) {
    Worker* worker = new Worker(std::move(job), [this]() { processQueue(); });
    if (!QThreadPool::globalInstance()->tryStart(worker)) {
        qCritical() << "Could not start thread";
    }
}