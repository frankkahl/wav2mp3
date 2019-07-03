#include "mutex.h"
#include <pthread.h>

pthread::mutex::mutex() noexcept { pthread_mutex_init(&_mutex, NULL); }
pthread::mutex::~mutex() {
    unlock();
    pthread_mutex_destroy(&_mutex);
}

void pthread::mutex::lock() { pthread_mutex_lock(&_mutex); }

void pthread::mutex::unlock() { pthread_mutex_unlock(&_mutex); }
