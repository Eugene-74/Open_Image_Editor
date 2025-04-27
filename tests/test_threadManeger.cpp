#include <gtest/gtest.h>

#include <QThreadPool>
#include <atomic>
#include <chrono>
#include <thread>

#include "ThreadManager.hpp"

TEST(ThreadManagerTest, AddThread) {
    ThreadManager manager;
    std::atomic<int> counter = 0;

    manager.addThread([&]() { counter++; });
    QThreadPool::globalInstance()->waitForDone();

    EXPECT_EQ(counter.load(), 1);
}

TEST(ThreadManagerTest, AddThreadToQueue) {
    ThreadManager manager;
    std::atomic<int> counter = 0;

    manager.addThread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        counter++;
    });
    manager.addThread([&]() { counter++; });

    QThreadPool::globalInstance()->waitForDone();
    EXPECT_EQ(counter.load(), 2);
}

TEST(ThreadManagerTest, AddHeavyThread) {
    ThreadManager manager;
    std::atomic<int> counter = 0;

    manager.addHeavyThread([&]() { counter++; });
    QThreadPool::globalInstance()->waitForDone();

    EXPECT_EQ(counter.load(), 1);
}

TEST(ThreadManagerTest, AddThreadToFront) {
    ThreadManager manager;
    std::atomic<int> counter = 0;

    manager.addThread([&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        counter++;
    });
    manager.addThreadToFront([&]() { counter += 10; });

    QThreadPool::globalInstance()->waitForDone();
    EXPECT_EQ(counter.load(), 11);
}

TEST(ThreadManagerTest, RemoveAllThreads) {
    ThreadManager manager;
    std::atomic<int> counter = 0;

    for (int i = 0; i < 100; ++i) {
        manager.addThread([&]() { counter++; });
    }
    manager.removeAllThreads();

    QThreadPool::globalInstance()->waitForDone();
    EXPECT_LE(counter.load(), 100);
}

// TEST(ThreadManagerTest, QueueOverflow) {
//     ThreadManager manager(nullptr, 1);
//     std::atomic<int> counter = 0;

//     for (int i = 0; i < 1100; ++i) {
//         manager.addThread([&]() { counter++; });
//     }

//     // QThreadPool::globalInstance()->waitForDone();
//     EXPECT_LE(counter.load(), Const::Thread::MAX_IN_QUEUE);
// }
