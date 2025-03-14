#pragma once

#include <QObject>
#include <QThread>
#include <QVector>
#include <functional>
#include <queue>
class Worker : public QObject {
    Q_OBJECT

   public:
    using Task = std::function<void()>;
    using Callback = std::function<void()>;

    Worker(Task job, Callback callback = nullptr);

   public slots:
    void process();

   signals:
    void finished();

   private:
    Task job;
    Callback callback;
};

class ThreadManager : public QObject {
    Q_OBJECT

   public:
    explicit ThreadManager(QObject *parent = nullptr, int maxThreads = 1);
    ~ThreadManager();

    void addThread(std::function<void()> job, std::function<void()> callback = nullptr);
    void removeThread(int index);
    void removeAllThreads();
    int getThreadCount() const;

   private:
    void startNextThread();

    QVector<QThread *> threads;
    int maxThreads;
    std::queue<std::pair<std::function<void()>, std::function<void()>>> taskQueue;
};
