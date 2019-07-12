#ifndef SIMPLELIB_THREAD_MODEL_HPP_
#define SIMPLELIB_THREAD_MODEL_HPP_

#include <atomic>
#include <thread>
#include <memory>
#include <future>
#include <ostream>
#include <functional>
#include <unordered_map>
#include <queue>
#include <chrono>
#include <pthread.h>

#include "common.h"

BEGIN_NAMESPACE_SIMPLELIB

class ThreadBase {
 public:
  virtual void Start(void* args) = 0;
  virtual void Stop() = 0;
  virtual void Join() = 0;
  virtual void Kill() = 0; // Kill to force stop

 protected:
  virtual void Run(void* args) = 0;
  std::thread t_;
};

class ThreadStoppable : public ThreadBase {
 public:
  virtual void Start(void* args) {
    bool old_val = true;
    if (!stop_.compare_exchange_strong(old_val, false)) { // use C-E-S to check once
      // Already started
      return;
    }

    std::thread temp([this, args] { Run(args); });
    t_.swap(temp);
    auto handle = t_.native_handle();
    native_handle_.store(handle, std::memory_order_release);
  }

  virtual void Stop() {
    bool old_val = false;
    while(!stop_.compare_exchange_weak(old_val, true) && !old_val); // do util success
  }

  virtual void Join() {
    if (t_.joinable()) {
      t_.join();
      native_handle_ = 0;
    }
  }

  virtual void Kill() {
    if (native_handle_.load(std::memory_order_acquire)) {
      pthread_cancel(native_handle_);
      native_handle_.store(0, std::memory_order_release);
    }
  }

  virtual void Suspend() {
    if (stop_.load(std::memory_order_acquire)) {
      return;
    }

    bool old_val = false;
    pause_.compare_exchange_strong(old_val, true);
  }

  virtual void Resume() {
    if (stop_.load(std::memory_order_acquire)) {
      return;
    }

    bool old_val = true;
    pause_.compare_exchange_strong(old_val, false);
  }

 protected:
  // stop_ is a sign telling if thread is destroyed
  // At begining it is true.
  std::atomic<bool> stop_{true};

  // pause_ is a sign telling if loop is suspended
  std::atomic<bool> pause_{false};

  // native handle will be used by kill api
  std::atomic<pthread_t> native_handle_{0};
};

END_NAMESPACE_SIMPLELIB

#endif // SIMPLELIB_THREAD_MODEL_HPP_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
