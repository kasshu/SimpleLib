#ifndef SIMPLELIB_TIMER_HPP_
#define SIMPLELIB_TIMER_HPP_

#include <atomic>
#include <chrono>
#include <condition_variable>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <unordered_map>

#include "common.h"
#include "thread_model.hpp"

BEGIN_NAMESPACE_SIMPLELIB

constexpr uint32_t kThreadTimeoutMS = 100;
constexpr uint32_t kThreadMinTimeoutMS = 10;

// A light weight timer implemented with a stand-alone thread and priority queue
// You should never put a time-consuming task into it,
// otherwise the task may delay other tasks significantly
class HeapTimer : public ThreadStoppable {
public:
    typedef uint64_t TaskId;

    enum TaskStatus {
        kWaiting = 0,
        kRunning,
        kCanceled
    };

    struct Task {
        TaskId id_;
        std::function<void()> callback_;
        uint64_t time_point_ms_;
        std::atomic<TaskStatus> status_{kWaiting};
    };

    // -1: Failed
    //  0: Success
    int Schedule(std::function<void()> callback, uint64_t timeout_ms, TaskId* task_id) {
        if (!callback || timeout_ms < min_timeout_.load(std::memory_order_acquire) || task_id == nullptr) {
            return -1;
        }

        auto task = std::make_shared<Task>();
        task->id_ = next_task_id_.fetch_add(1, std::memory_order_acq_rel);
        task->callback_ = callback;
        task->time_point_ms_ = steady_clock_now_ms() + timeout_ms;

        {
            std::lock_guard<std::mutex> lock(mutex_);
            task_map_.emplace(task->id_, task);
            task_queue_.emplace(task);
            // Check if need to reschedule
            if (wait_timeout_.load(std::memory_order_acquire) > timeout_ms) {
                cond_.notify_one();
            }
        }

        *task_id = task->id_;

        return 0;
    }

    // -1: No such task
    //  0: Success, task has not been run yet
    //  1: Ops! Task is running now, you cannot cancel it
    int Cancel(TaskId task_id) {
        std::shared_ptr<Task> task;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = task_map_.find(task_id);
            if (it == task_map_.end()) {
                return -1;
            }
            task = it->second;
        }

        TaskStatus old_status = kWaiting;
        if (task->status_.compare_exchange_strong(old_status, kCanceled)) {
            return 0; // Old status is kWaiting
        } else { 
            if (old_status == kCanceled) {
                return 0; // Old status is kCanceled
            } else {
                return 1; // Old status is kRunning
            }
        }
    }

  void SetMinTimeout(uint32_t timeout_ms) {
    uint32_t temp = std::max(timeout_ms, kThreadMinTimeoutMS);
    min_timeout_.store(temp, std::memory_order_release);
  }

  virtual void Stop(){
    ThreadStoppable::Stop();
    cond_.notify_one();
  }

 protected:
  static bool TaskTimePointLater(const std::shared_ptr<Task>& left, const std::shared_ptr<Task>& right) {
    return left->time_point_ms_ > right->time_point_ms_;
  }

  static uint64_t steady_clock_now_ms() {
      return std::chrono::duration_cast<std::chrono::milliseconds>
              (std::chrono::steady_clock::now().time_since_epoch()).count();
  }

  virtual bool ConsumeTasks(std::vector<std::shared_ptr<Task>>* task_vec) {
    auto timeout = wait_timeout_.load(std::memory_order_acquire);
    std::unique_lock<std::mutex> lock(mutex_);
    cond_.wait_for(lock, std::chrono::milliseconds(timeout));
    if (stop_.load(std::memory_order_acquire)) {
      return false;
    }

    auto now = steady_clock_now_ms();
    while (!task_queue_.empty()) {
      auto task = task_queue_.top();
      if (task->time_point_ms_ > now) {
        break;
      }

      task_vec->push_back(task);
      task_queue_.pop();
    }

    return true;
  }

  virtual void GetTopTimePoint(uint64_t* top_time_point_ms) {
    std::unique_lock<std::mutex> lock(mutex_);
    if (!task_queue_.empty()) {
      auto task = task_queue_.top();
      *top_time_point_ms = task->time_point_ms_;
    }
  }

  virtual void RemoveTaskFromMap(TaskId id) {
    std::unique_lock<std::mutex> lock(mutex_);
    task_map_.erase(id);
  }

  virtual void Run(void* args) {
    (void)args;
    while (!stop_.load(std::memory_order_acquire)) {
      std::vector<std::shared_ptr<Task>> task_vec;
      bool continue_to_run = ConsumeTasks(&task_vec);
      if (!continue_to_run) {
        break;
      }

      for (auto it = task_vec.begin(); it != task_vec.end(); it++) {
        auto task = *it;
        TaskStatus old_status = kWaiting;
        if (task->status_.compare_exchange_strong(old_status, kRunning)) {
          task->callback_();
        } // else, status == kCanceled
        RemoveTaskFromMap(task->id_);
      }

      uint64_t top_time_point_ms = std::numeric_limits<uint64_t>::max();
      GetTopTimePoint(&top_time_point_ms);

      if (top_time_point_ms == std::numeric_limits<uint64_t>::max()) {
        // No task in queue, wait for default timeout
        wait_timeout_.store(kThreadTimeoutMS, std::memory_order_release);
      } else {
        auto now = steady_clock_now_ms();
        if (now > top_time_point_ms) {
          wait_timeout_.store(0, std::memory_order_release); // Timeout immediately
        } else {
          wait_timeout_.store(top_time_point_ms - now, std::memory_order_release) ;
        }
      }
    } // end of while loop
  } // end of Run function

  std::atomic<uint64_t> wait_timeout_{kThreadTimeoutMS};
  std::atomic<uint64_t> min_timeout_{kThreadMinTimeoutMS};
  std::atomic<TaskId> next_task_id_{0};
  std::condition_variable cond_;
  std::mutex mutex_;
  std::unordered_map<TaskId, std::shared_ptr<Task>> task_map_;
  std::priority_queue<std::shared_ptr<Task>,
                      std::vector<std::shared_ptr<Task>>,
                      decltype(&TaskTimePointLater)> task_queue_{&TaskTimePointLater};
};


END_NAMESPACE_SIMPLELIB

#endif  //SIMPLELIB_TIMER_HPP_

/* vim: set ts=4 sw=4 sts=4 tw=120 noet: */
