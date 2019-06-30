#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "thread_queue.h"

#include "thread_includes.h"

#include <cstdint>
#include <functional>
#include <memory>
#include <vector>

// implements a pool of threads
// allows to queue a function to be executed by a worker thread
// using the "enqueue" method
// The queuing method "enqueue" blocks until one of the worker threads gets idle
// and is ready to immediately process the function

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
    void enqueue(std::function<void(const std::uint16_t)> function_to_execute);

   private:  // private typedefs
             // struct which stores the actual std::thread and a queue for communicating
             // with it
    typedef struct Thread {
        std::shared_ptr<std::thread> thread;
        ThreadQueue<std::function<void(const std::uint16_t)> > queue;
    } Thread;

   private:  // private methods
    typedef struct ThreadArguments {
        ThreadPool *thread_pool;
        std::uint16_t thread_number;
    } ThreadArguments;
    // static thread function to execute in a worker thread. The number of the particular thread is passed as an argument
    static void *thread_function(void *arg_ptr);
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
    condition_variable _thread_args_copied;         // used to signal the condition that the thread function has copied the content
                                                    // of the passed ThreadArguments instance so that it can go out of scope
    mutex _thread_args_copied_mutex;                // mutex to use in conjunction with _thread_args_copied


};

#endif  // THREADPOOL_H
