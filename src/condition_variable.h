#ifndef CONDITION_VARIABLE_H
#define CONDITION_VARIABLE_H

#include "mutex.h"
#include <pthread.h>

namespace pthread {
// minimum functionality of the mutex class of the C++ 11 threading library
// required for this application reimplemented using
class condition_variable {

   public:
    condition_variable() noexcept;
    condition_variable(const condition_variable&) = delete;
    ~condition_variable();

    condition_variable& operator=(const condition_variable&) = delete;
    void wait(pthread::unique_lock<pthread::mutex>& lock);
    void lock();
    void unlock();
    void notify_one();

   private:
  pthread_cond_t _cond_var;
};
}

#endif  // CONDITION_VARIABLE_H
