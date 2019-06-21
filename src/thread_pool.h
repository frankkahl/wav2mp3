#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "thread_queue.h"

#include <condition_variable>
#include <cstdint>
#include <future>
#include <memory>
#include <thread>
#include <vector>
#include <functional>


// implements a pool of threads
// allows to queue a function to be executed by a worker thread
// using the "enqueue" method
// The queuing method "enqueue" blocks until one of the worker threads gets idle
// and is ready to immediately process the function

template <typename T>
class ThreadPool {
   public:
    ThreadPool(const std::uint16_t num_of_threads = std::thread::hardware_concurrency());
    virtual ~ThreadPool();

    // enqueues a function pointer "function_to_execute" to be executed by the next available
    // worker thread. The "function_to_execute" must take as a single argument
    // the number of thread from which it is executed and return a result
    // of type T
    // if all threads are currently busy the method waits until one becomes idle
    // returns a future to this result
    std::future<T> enqueue(std::function<T(const std::uint16_t)> function_to_execute);

   private:  // private typedefs
             // struct which stores the actual std::thread, a queue for communicating
             // with it and the promise to store the result the function executed
             // by the thread
    typedef struct Thread {
        std::shared_ptr<std::thread> thread;
        ThreadQueue<std::function<T(const std::uint16_t)> > queue;
        std::promise<T> promise;
    } Thread;

   private:  // private methods
    // thread method to execute in a worker thread. The number of the particular thread is passed as an argument
    void thread_method(const std::uint16_t thread_number);
    // returns the next idle worker thread. Waits until one becomes idle if necessary
    // returns the number of the idle thread
    int get_next_idle_thread();
    // meant to be called from inside a thread to mark the thread as idle and ready to process the next task
    void notify_being_idle(const std::uint16_t thread_number);
    // starts all threads
    // helper method for the constructor
    void start_all_threads();
    // request all threads to stop and then joins() them
    // helper method for the destructor
    void stop_all_threads();

   private:                                         // private data
    std::vector<Thread> _threads;                   // stores Thread instance for each worker thread
                                                    // the index of the vector is used as the
                                                    // thread number
    ThreadQueue<unsigned int> _idle_threads_queue;  // used by the threads to send their number
                                                    // to the main thread when they become idle
};

#include "thread_pool_impl.h"

#endif  // THREADPOOL_H
