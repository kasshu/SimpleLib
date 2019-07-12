#ifndef SIMPLELIB_SINGLETON_HPP_
#define SIMPLELIB_SINGLETON_HPP_

#include <mutex>
#include <atomic>
#include "common.h"

BEGIN_NAMESPACE_SIMPLELIB

//A thread safe and DCLP singleton using CRTP
template <typename Derived>
class Singleton {
public:
    static Derived * getInstance() {
        Derived * temp = m_instance.load(std::memory_order_acquire);
        if (temp == nullptr) {
            std::lock_guard<std::mutex> lock(m_mutex);
            temp = m_instance.load(std::memory_order_relaxed);
            if (temp == nullptr) {
                temp = new Derived;
                m_instance.store(temp, std::memory_order_release);
            }
        }
        return temp;
    }
protected:
    Singleton() = default;
    ~Singleton() = default;
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
private:
    static std::atomic<Derived *> m_instance;
    static std::mutex m_mutex;
};

template<typename Derived>
std::atomic<Derived *> Singleton<Derived>::m_instance(nullptr);

template<typename Derived>
std::mutex Singleton<Derived>::m_mutex;

END_NAMESPACE_SIMPLELIB

#endif  //SIMPLELIB_SINGLETON_HPP_

/* vim: set ts=4 sw=4 sts=4 tw=100 noet: */
