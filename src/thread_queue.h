#ifndef THREAD_QUEUE_H
#define THREAD_QUEUE_H

#include "thread_includes.h"

#include <queue>

// a thread-save queue for passing instances of type T from the main
// thread to a worker thread

template <typename T>
class ThreadQueue {
   public:
    // pushed the instance "element" of type T into the queue
    void enqueue(T element);
    // waits until at least one elemnt is in the queue and
    T dequeue();

   private:
    std::queue<T> _queue;
    std::mutex _mutex;
    std::condition_variable _cond_var;
};

#include "thread_queue_impl.h"

#endif  // THREAD_QUEUE_H
