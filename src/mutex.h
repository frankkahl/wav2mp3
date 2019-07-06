#ifndef MUTEX_H
#define MUTEX_H

#include <pthread.h>

namespace pthread {
// minimum functionality of the mutex class of the C++ 11 threading library
// required for this application
// implemented using the pthreads libaray

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
    pthread_mutex_t _mutex;
};

// Alias unique_lock introduced since the C++ thread interface requires
// to pass a mutex lock guard of type unique_lock<mutex> to be passed to
// the wait() method of condition_variable
template <typename mutex_type>
using unique_lock = lock_guard<mutex_type>;

}  // namespace pthread
#endif  // MUTEX_H
