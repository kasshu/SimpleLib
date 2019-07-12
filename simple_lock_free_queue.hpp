#ifndef SIMPLELIB_LOCK_FREE_QUEUE_HPP_
#define SIMPLELIB_LOCK_FREE_QUEUE_HPP_

#include <atomic>
#include <thread>
#include <type_traits>
#include "common.h"

#define SLFQ_UNLIKELY(x) __builtin_expect(!!(x), 0)

BEGIN_NAMESPACE_SIMPLELIB

// This is a trade-off between space and execution time
// We use more space to avoid false sharing between producers
template<typename T>
struct SimpleLockFreeQueueElement {
  alignas(64) std::atomic<int64_t> flag;
  typename std::aligned_storage<sizeof(T), alignof(T)>::type data;
};

constexpr uint32_t kSimpleLockFreeQueueDefaultSize = 16384; // 16k

// A "cheap" version disruptor:
// 1. Ring buffer
// 2. Using CAS instead of mutex
// 3. Alignment to 64 bytes to avoiding cache line false sharing
// 4. Force size to be power of 2 to speed up element locating
//
// Hits:
// When you want to stop, you MUST call Invalid() to save your threads from infinite looping in Push/Pop/Emplace
template<typename T, uint32_t SIZE = kSimpleLockFreeQueueDefaultSize>
class SimpleLockFreeQueue
{
 public:
  SimpleLockFreeQueue() {
    for(uint32_t i = 0; i < SIZE; i++) {
      ring_buffer_[i].flag.store(i, std::memory_order_relaxed);
    }
  }

  ~SimpleLockFreeQueue() {
    for(uint32_t i = 0; i < SIZE; i++) {
      if(ring_buffer_[i].flag.load(std::memory_order_relaxed) < 0) {
        reinterpret_cast<T&>(ring_buffer_[i].data).~T();
      }
    }
  }

  template<typename... Args>
  bool Emplace(Args&&... args) {
    if (SLFQ_UNLIKELY(!IsValid())) {
      return false;
    }

    if (IsFull()) {
      return false;
    }

    int64_t current_idx = write_idx_.fetch_add(1, std::memory_order_relaxed);
    auto& elem = ring_buffer_[current_idx & round_];
    while (elem.flag.load(std::memory_order_acquire) != current_idx) {
      if (SLFQ_UNLIKELY(!IsValid())) {
        return false;
      }
      std::this_thread::yield();
    }
    new(&elem.data) T(std::forward<Args>(args)...);
    elem.flag.store(~current_idx, std::memory_order_release);

    return true;
  }

  bool Push(const T& t) {
    return Emplace(t);
  }

  bool Pop(T* t) {
    if (SLFQ_UNLIKELY(!IsValid())) {
      return false;
    }

    if (IsEmpty()) {
      return false;
    }

    int64_t current_idx = read_idx_.fetch_add(1, std::memory_order_relaxed);
    auto& elem = ring_buffer_[current_idx & round_];
    while (elem.flag.load(std::memory_order_acquire) != ~current_idx) {
      if (SLFQ_UNLIKELY(!IsValid())) {
        return false;
      }
      std::this_thread::yield();
    }
    *t = std::move(*reinterpret_cast<const T*>(&elem.data));
    elem.flag.store(current_idx + SIZE, std::memory_order_release);

    return true;
  }

  // You MUST call Invalid to save your threads from infinite looping in Push/Pop/Emplace
  void Invalid() {
    return valid_.store(false,std::memory_order_relaxed);
  }

  int64_t Size() {
    return write_idx_.load(std::memory_order_relaxed) - read_idx_.load(std::memory_order_relaxed);
  }

  bool IsEmpty() {
      return Size() <= 0;
  }

  bool IsFull() {
      return Size() >= SIZE;
  }

  bool IsValid() {
    return valid_.load(std::memory_order_relaxed);
  }

 private:
  // Compile time constants
  static_assert(SIZE && !(SIZE & (SIZE - 1)), "SIZE must be a power of 2");
  constexpr static uint32_t round_ = SIZE - 1;

  // Indexes
  alignas(64) std::atomic<int64_t> write_idx_{0};
  alignas(64) std::atomic<int64_t> read_idx_{0};

  // Stop flag
  std::atomic<bool> valid_{true};

  // Ring buffer
  SimpleLockFreeQueueElement<T> ring_buffer_[SIZE];
};

END_NAMESPACE_SIMPLELIB

#endif // SIMPLELIB_LOCK_FREE_QUEUE_HPP_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
