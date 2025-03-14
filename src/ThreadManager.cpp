#include "ThreadManager.hpp"

#include <QDebug>

// ----- Worker implementation -----
Worker::Worker(Task job, Callback callback)
    : job(std::move(job)), callback(std::move(callback)) {}

void Worker::process() {
    if (job) job();
    if (callback) callback();
    emit finished();
}

// ----- ThreadManager implementation -----
ThreadManager::ThreadManager(QObject* parent, int maxThreads)
    : QObject(parent), maxThreads(maxThreads) {}

ThreadManager::~ThreadManager() {
    for (QThread* thread : threads) {
        thread->quit();
        thread->wait();
        delete thread;
    }
}

void ThreadManager::addThread(std::function<void()> job, std::function<void()> callback) {
    taskQueue.push({std::move(job), std::move(callback)});
    startNextThread();
}

void ThreadManager::startNextThread() {
    if (threads.size() >= maxThreads || taskQueue.empty()) {
        return;
    }

    auto [job, callback] = taskQueue.front();
    taskQueue.pop();

    QThread* thread = new QThread;
    Worker* worker = new Worker(std::move(job), std::move(callback));

    worker->moveToThread(thread);

    // Connexions du thread au worker
    connect(thread, &QThread::started, worker, &Worker::process);
    connect(worker, &Worker::finished, thread, &QThread::quit);
    connect(worker, &Worker::finished, worker, &Worker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    // Connexion pour supprimer le thread de la liste lorsqu'il est terminé
    connect(thread, &QThread::finished, this, [this, thread]() {
        threads.removeOne(thread);
        startNextThread();
    });

    threads.append(thread);
    thread->start();
}

void ThreadManager::removeThread(int index) {
    if (index >= 0 && index < threads.size()) {
        QThread* thread = threads.at(index);
        qDebug() << "Arrêt du thread" << index;
        thread->quit();
        thread->wait();
        threads.removeAt(index);
    }
}

void ThreadManager::removeAllThreads() {
    for (QThread* thread : threads) {
        thread->quit();
        thread->wait();
    }
    while (!taskQueue.empty()) {
        taskQueue.pop();
    }
    threads.clear();
}

int ThreadManager::getThreadCount() const {
    return threads.size();
}
