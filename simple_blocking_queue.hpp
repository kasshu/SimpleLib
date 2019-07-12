#ifndef SIMPLELIB_BLOCKING_QUEUE_HPP_
#define SIMPLELIB_BLOCKING_QUEUE_HPP_

#include <atomic>
#include <deque>
#include <mutex>
#include <memory>
#include <chrono>
#include <limits>
#include <cstdint>
#include <functional>
#include <condition_variable>
#include "common.h"

BEGIN_NAMESPACE_SIMPLELIB

template<typename T,
    template<typename ELEM, typename ALLOC = std::allocator<ELEM>>
    class CONT = std::deque>
class SimpleBlockingQueue {
 public:
  SimpleBlockingQueue() = default;
  explicit SimpleBlockingQueue(std::uint32_t max_size) : max_size_(max_size) {}
  virtual ~SimpleBlockingQueue() {
    queue_.clear();
  }

  void PushBack(const T &t) {
    std::unique_lock<std::mutex> locker(mutex_);
    not_full_.wait(locker, [this]() { return queue_.size() < max_size_.load(); });
    queue_.push_back(t);
    not_empty_.notify_one();
  }

  void PushFront(const T &t) {
    std::unique_lock<std::mutex> locker(mutex_);
    not_full_.wait(locker, [this]() { return queue_.size() < max_size_.load(); });
    queue_.push_front(t);
    not_empty_.notify_one();
  }

  bool PushBackWithTimeout(const T &t, int timeout/*in milliseconds*/) {
    std::unique_lock<std::mutex> locker(mutex_);
    if (not_full_.wait_for(locker,
                           std::chrono::milliseconds(timeout),
                           [this]() { return queue_.size() < max_size_.load(); })) {
      queue_.push_back(t);
      not_empty_.notify_one();
      return true;
    }
    //timeout
    return false;
  }

  bool PushFrontWithTimeout(const T &t, int timeout/*in milliseconds*/) {
    std::unique_lock<std::mutex> locker(mutex_);
    if (not_full_.wait_for(locker,
                           std::chrono::milliseconds(timeout),
                           [this]() { return queue_.size() < max_size_.load(); })) {
      queue_.push_front(t);
      not_empty_.notify_one();
      return true;
    }
    //timeout
    return false;
  }

  template<typename... Args>
  void EmplaceBack(Args &&... args) {
    std::unique_lock<std::mutex> locker(mutex_);
    not_full_.wait(locker, [this]() { return queue_.size() < max_size_.load(); });
    queue_.emplace_back(std::forward<Args>(args)...);
    not_empty_.notify_one();
  }

  template<typename... Args>
  void EmplaceFront(Args &&... args) {
    std::unique_lock<std::mutex> locker(mutex_);
    not_full_.wait(locker, [this]() { return queue_.size() < max_size_.load(); });
    queue_.emplace_front(std::forward<Args>(args)...);
    not_empty_.notify_one();
  }

  template<typename... Args>
  bool EmplaceBackWithTimeout(int timeout/*in milliseconds*/, Args &&... args) {
    std::unique_lock<std::mutex> locker(mutex_);
    if (not_full_.wait_for(locker,
                           std::chrono::milliseconds(timeout),
                           [this]() { return queue_.size() < max_size_.load(); })) {
      queue_.emplace_back(std::forward<Args>(args)...);
      not_empty_.notify_one();
      return true;
    }
    //timeout
    return false;
  }

  template<typename... Args>
  bool EmplaceFrontWithTimeout(int timeout/*in milliseconds*/, Args &&... args) {
    std::unique_lock<std::mutex> locker(mutex_);
    if (not_full_.wait_for(locker,
                           std::chrono::milliseconds(timeout),
                           [this]() { return queue_.size() < max_size_.load(); })) {
      queue_.emplace_front(std::forward<Args>(args)...);
      not_empty_.notify_one();
      return true;
    }
    //timeout
    return false;
  }

  void PopFront(T *t) {
    std::unique_lock<std::mutex> locker(mutex_);
    not_empty_.wait(locker, [this]() { return !queue_.empty(); });
    (*t) = std::move(queue_.front());
    queue_.pop_front();
    not_full_.notify_one();
  }

  void PopBack(T *t) {
    std::unique_lock<std::mutex> locker(mutex_);
    not_empty_.wait(locker, [this]() { return !queue_.empty(); });
    (*t) = std::move(queue_.front());
    queue_.pop_back();
    not_full_.notify_one();
  }

  bool PopFrontWithTimeout(T *t, int timeout/*in milliseconds*/) {
    std::unique_lock<std::mutex> locker(mutex_);
    if (not_empty_.wait_for(locker,
                            std::chrono::milliseconds(timeout),
                            [this]() { return !queue_.empty(); })) {
      (*t) = std::move(queue_.front());
      queue_.pop_front();
      not_full_.notify_one();
      return true;
    }
    //timeout
    return false;
  }

  bool PopBackWithTimeout(T *t, int timeout/*in milliseconds*/) {
    std::unique_lock<std::mutex> locker(mutex_);
    if (not_empty_.wait_for(locker,
                            std::chrono::milliseconds(timeout),
                            [this]() { return !queue_.empty(); })) {
      (*t) = std::move(queue_.front());
      queue_.pop_back();
      not_full_.notify_one();
      return true;
    }
    //timeout
    return false;
  }

  void SetMaxSize(uint32_t max_size) {
    max_size_.store(max_size);
  }

  void Clear() {
    std::lock_guard<std::mutex> locker(mutex_);
    queue_.clear();
  }

  size_t Size() {
    std::lock_guard<std::mutex> locker(mutex_);
    return queue_.size();
  }

  bool Empty() {
    std::lock_guard<std::mutex> locker(mutex_);
    return queue_.empty();
  }
 private:
  CONT<T> queue_;
  std::mutex mutex_;
  std::condition_variable not_empty_;
  std::condition_variable not_full_;
  std::atomic<uint32_t> max_size_{std::numeric_limits<std::uint32_t>::max()};
};

END_NAMESPACE_SIMPLELIB

#endif // SIMPLELIB_BLOCKING_QUEUE_HPP_
