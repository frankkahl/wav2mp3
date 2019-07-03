#include "condition_variable.h"
#include "mutex.h"
#include <pthread.h>


pthread::condition_variable::condition_variable() noexcept { pthread_cond_init(&_cond_var, NULL); }
pthread::condition_variable::~condition_variable() { pthread_cond_destroy(&_cond_var); }
void pthread::condition_variable::wait(pthread::unique_lock<pthread::mutex>& lock) { pthread_cond_wait(&_cond_var, &lock._mutex._mutex); }
void pthread::condition_variable::notify_one() { pthread_cond_signal(&_cond_var); }
