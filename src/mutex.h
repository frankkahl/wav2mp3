#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>

namespace std {
// minimum functionality of the mutex class of the C++ 11 threading library
// required for this application reimplemented using

class condition_variable;

template <typename mutex_type>
class lock_guard {
    friend condition_variable;

   public:
    lock_guard(mutex_type& mutex) : _mutex(mutex) { _mutex.lock(); }
    ~lock_guard() { _mutex.unlock(); }

   private:
    mutex_type& _mutex;
};

class mutex {
    friend condition_variable;

   public:
    mutex() noexcept;
    ~mutex();

    mutex(const mutex&) = delete;
    mutex& operator=(const mutex&) = delete;
    void lock();
    void unlock();

   private:
    pthread_mutex_t _mutex = nullptr;
};

template <typename mutex_type>
using unique_lock = lock_guard<mutex_type>;
}  // namespace std
#endif  // MUTEX_H