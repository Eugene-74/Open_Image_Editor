#include "ThreadManager.hpp"

#include <QDebug>
#include <QThreadPool>
#include <queue>

#include "Const.hpp"

// ----- Worker implementation -----
Worker::Worker(std::function<void()> job)
    : job(std::move(job)) {}

void Worker::run() {
    if (job) job();
}

// ----- ThreadManager implementation -----
ThreadManager::ThreadManager(QObject* parent, int maxThreads)
    : QObject(parent), maxThreads(maxThreads) {
    QThreadPool::globalInstance()->setMaxThreadCount(maxThreads);
}

ThreadManager::~ThreadManager() {
    QThreadPool::globalInstance()->waitForDone();
}

void ThreadManager::addHeavyThread(std::function<void()> job) {
    heavyTaskQueue.push_back(std::move(job));
    qInfo() << "Job added to heavy task queue. Queue size: " << heavyTaskQueue.size();
}
void ThreadManager::addThread(std::function<void()> job) {
    if (QThreadPool::globalInstance()->activeThreadCount() < maxThreads) {
        Worker* worker = new Worker(std::move(job));
        QThreadPool::globalInstance()->start(worker);
        qInfo() << "Thread pool active thread count: " << QThreadPool::globalInstance()->activeThreadCount();
        qInfo() << "Thread pool max thread count: " << QThreadPool::globalInstance()->maxThreadCount();
    } else {
        taskQueue.push_back(std::move(job));
        qInfo() << "Job added to queue. Queue size: " << taskQueue.size();
    }
}

void ThreadManager::addThreadToFront(std::function<void()> job) {
    taskQueue.push_front(std::move(job));
    qInfo() << "Job added to front of queue. Queue size: " << taskQueue.size();
}

void ThreadManager::removeAllThreads() {
    QThreadPool::globalInstance()->clear();
    taskQueue.clear();
    heavyTaskQueue.clear();
}

int ThreadManager::getThreadCount() const {
    return QThreadPool::globalInstance()->activeThreadCount();
}

void ThreadManager::processQueue() {
    while (!taskQueue.empty() || !heavyTaskQueue.empty()) {
        if (QThreadPool::globalInstance()->activeThreadCount() >= maxThreads) {
            return;
        }
        if (!taskQueue.empty()) {
            std::function<void()> job = std::move(taskQueue.front());
            taskQueue.pop_front();
            addThread(std::move(job));
        } else if (QThreadPool::globalInstance()->activeThreadCount() <= maxThreads - FREE_THREAD) {
            std::function<void()> job = std::move(heavyTaskQueue.front());
            heavyTaskQueue.pop_front();
            addThread(std::move(job));
        }
    }
}