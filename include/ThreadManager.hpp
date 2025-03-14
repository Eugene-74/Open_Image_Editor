#pragma once

#include <QObject>
#include <QRunnable>
#include <QThreadPool>
#include <deque>
#include <functional>
#include <queue>

class Worker : public QRunnable {
   public:
    Worker(std::function<void()> job);
    void run() override;

   private:
    std::function<void()> job;
};

class ThreadManager : public QObject {
    Q_OBJECT

   public:
    ThreadManager(QObject* parent = nullptr, int maxThreads = std::max(QThreadPool::globalInstance()->maxThreadCount() - 2, 1));
    ~ThreadManager();

    void addThread(std::function<void()> job);
    void addHeavyThread(std::function<void()> job);
    void addThreadToFront(std::function<void()> job);
    void removeAllThreads();
    int getThreadCount() const;

   private:
    int maxThreads;
    std::deque<std::function<void()>> taskQueue;
    std::deque<std::function<void()>> heavyTaskQueue;

    void processQueue();
};