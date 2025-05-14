#pragma once

#include <QObject>
#include <QRunnable>
#include <QThreadPool>
#include <deque>
#include <functional>
#include <queue>

#include "Const.hpp"

class Worker : public QRunnable {
   public:
    Worker(std::function<void()> job, std::function<void()> process);
    void run() override;

    std::function<void()> job;
    std::function<void()> process;
};

class ThreadManager : public QObject {
    Q_OBJECT

   public:
    ThreadManager(QObject* parent = nullptr, int maxThreads = QThreadPool::globalInstance()->maxThreadCount());
    ~ThreadManager();

    void addThread(std::function<void()> job);
    void addThreadToFront(std::function<void()> job);

    void addHeavyThread(std::function<void()> job);
    void addHeavyThreadToFront(std::function<void()> job);

    void removeAllThreads();
    int getThreadCount() const;

   private:
       void startJob(std::function<void()> job);
    int maxThreads;
    std::deque<std::function<void()>> taskQueue;
    std::deque<std::function<void()>> heavyTaskQueue;

    std::mutex taskQueueMutex;
    std::mutex heavyTaskQueueMutex;

    void processQueue();
};