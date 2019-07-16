#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <cstdint>

class ThreadPool;

namespace pthread {
// minimum functionality of the thread class of the C++ 11 threading library
// required for this application
// implemented using the pthreads libaray

class thread {
  public:
    thread() noexcept;
    thread(const thread &) = delete;
    // here only the constructor signature required by the the pthread_create function is implemented,
    // not the much more generic template based constructor of the C++ threading library
    explicit thread(void *(*thread_function)(void *), void *arg_ptr);

    ~thread();

    void join();

    // should return the number of processor cores
    // Not sure if virtual cores due  to hyperthreading counts as well
    static unsigned int hardware_concurrency();

  private:
    pthread_t _invalid_pthread;
    pthread_t _thread;
};
}  // namespace pthread
#endif  // THREAD_H
