#ifndef CONDITION_VARIABLE_H
#define CONDITION_VARIABLE_H

#include "mutex.h"
#include <pthread.h>

namespace std {
// minimum functionality of the mutex class of the C++ 11 threading library
// required for this application reimplemented using
class condition_variable {

   public:
    condition_variable() noexcept;
    condition_variable(const condition_variable&) = delete;
    ~condition_variable();

    condition_variable& operator=(const condition_variable&) = delete;
    void wait(std::unique_lock<std::mutex>& lock);
    void lock();
    void unlock();
    void notify_one();

   private:
    pthread_cond_t _cond_var = nullptr;
};
}

#endif  // CONDITION_VARIABLE_H