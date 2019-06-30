#include "condition_variable.h"
#include "mutex.h"
#include <pthread.h>


std::condition_variable::condition_variable() noexcept { pthread_cond_init(&_cond_var, NULL); }
std::condition_variable::~condition_variable() { pthread_cond_destroy(&_cond_var); }
void std::condition_variable::wait(std::unique_lock<std::mutex>& lock) { pthread_cond_wait(&_cond_var, &lock._mutex._mutex); }
void std::condition_variable::notify_one() { pthread_cond_signal(&_cond_var); }