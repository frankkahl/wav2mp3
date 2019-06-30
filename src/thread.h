#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>
#include <cstdint>

class ThreadPool;

namespace std {


class thread {
   public:
    thread() noexcept;
    thread(const thread&) = delete;

    explicit thread(void *(*thread_function)(void *), void *arg_ptr)  {  // construct with _Fx(_Ax...)
        pthread_create(&_thread, nullptr, thread_function, arg_ptr);
    }

    ~thread();

    void join();

    static unsigned int hardware_concurrency();

   private:
    pthread_t _invalid_pthread;
    pthread_t _thread;
};
}  // namespace std
#endif  // THREAD_H