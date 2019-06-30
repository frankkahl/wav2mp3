#include "mutex.h"
#include <pthread.h>

std::mutex::mutex() noexcept { pthread_mutex_init(&_mutex, NULL); }
std::mutex::~mutex() {
    unlock();
    pthread_mutex_destroy(&_mutex);
}

void std::mutex::lock() { pthread_mutex_lock(&_mutex); }

void std::mutex::unlock() { pthread_mutex_unlock(&_mutex); }
