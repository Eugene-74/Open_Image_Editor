#pragma once
#include <mutex>
#include <vector>

template <typename T>
class AsyncVector : public std::vector<T> {
   private:
    mutable std::mutex mutex;

   public:
    using std::vector<T>::vector;

    void push_back(const T& value) {
        std::lock_guard<std::mutex> lock(mutex);
        std::vector<T>::push_back(value);
    }

    void push_back(T&& value) {
        std::lock_guard<std::mutex> lock(mutex);
        std::vector<T>::push_back(std::move(value));
    }

    T& operator[](size_t idx) {
        std::lock_guard<std::mutex> lock(mutex);
        return std::vector<T>::operator[](idx);
    }

    const T& operator[](size_t idx) const {
        std::lock_guard<std::mutex> lock(mutex);
        return std::vector<T>::operator[](idx);
    }
};
