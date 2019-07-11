#include "condition_variable.h"
#include <pthread.h>
#include "check_pthread_error.h"
#include "mutex.h"

pthread::condition_variable::condition_variable() noexcept {
    int res = pthread_cond_init(&_cond_var, NULL);
    check_pthread_error(res, "pthread_cond_init");
}
pthread::condition_variable::~condition_variable() {
    int res = pthread_cond_destroy(&_cond_var);
    check_pthread_error(res, "pthread_cond_destroy");
}
void pthread::condition_variable::wait(pthread::unique_lock<pthread::mutex>& lock) {
    int res = pthread_cond_wait(&_cond_var, &lock._mutex._mutex);
    check_pthread_error(res, "pthread_cond_wait");
}
void pthread::condition_variable::notify_one() {
    int res = pthread_cond_signal(&_cond_var);
    check_pthread_error(res, "pthread_cond_signal");
}
