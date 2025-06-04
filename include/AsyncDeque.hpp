#pragma once
#include <deque>
#include <mutex>

template <typename T>
class AsyncDeque : public std::deque<T> {
   private:
    mutable std::mutex mutex;

   public:
    using std::deque<T>::deque;

    void push_back(const T& value) {
        std::lock_guard<std::mutex> lock(mutex);
        std::deque<T>::push_back(value);
    }

    void push_front(const T& value) {
        std::lock_guard<std::mutex> lock(mutex);
        std::deque<T>::push_front(value);
    }

    void pop_back() {
        std::lock_guard<std::mutex> lock(mutex);
        std::deque<T>::pop_back();
    }

    void pop_front() {
        std::lock_guard<std::mutex> lock(mutex);
        std::deque<T>::pop_front();
    }

    T& front() {
        std::lock_guard<std::mutex> lock(mutex);
        return std::deque<T>::front();
    }

    T& back() {
        std::lock_guard<std::mutex> lock(mutex);
        return std::deque<T>::back();
    }

    bool empty() {
        std::lock_guard<std::mutex> lock(mutex);
        return std::deque<T>::empty();
    }

    size_t size() {
        std::lock_guard<std::mutex> lock(mutex);
        return std::deque<T>::size();
    }
};
