#include "thread_pool.h"
#include "signal_handler.h"
#include "tiostream.h"

#include <cstdint>
#include <iostream>
#include <sstream>

ThreadPool::ThreadPool(const std::uint16_t num_of_threads) : _threads(num_of_threads), _thread_number(0) {
    if (num_of_threads < 1) {
        std::ostringstream ss;
        ss << "num_of_threads (" << num_of_threads << ") must not be smaller than 1";
        throw std::invalid_argument(ss.str());
    }
    start_all_threads();
    std::cout << "Thread pool started with " << num_of_threads << " threads." << std::endl;
};

ThreadPool::~ThreadPool() {
    stop_all_threads();
    std::cout << "Thread pool stopped." << std::endl;
};

void ThreadPool::start_all_threads() {
    pthread::unique_lock<pthread::mutex> lock_args_copied_mutex(_thread_args_copied_mutex);
    for (std::uint16_t i = 0; i < _threads.size(); ++i) {
        _thread_number = i;
        _threads[i].thread.reset(new pthread::thread(reinterpret_cast<void *(*)(void *)>(&ThreadPool::thread_function), this));
        _thread_args_copied.wait(lock_args_copied_mutex);
    }
}

void ThreadPool::stop_all_threads() {
    // notify all threads to terminate by sending a nullptr
    for (auto &thread : _threads) {
        thread.queue.enqueue(nullptr);
    }
    // then join the threads. The join() method waits until the thread has terminated
    for (auto &thread : _threads) {
        thread.thread->join();
    }
}

void ThreadPool::enqueue(std::function<void(const std::uint16_t)> function_to_execute) {
    // get the number of a thread which is idle
    // wait if none is idle yet
    auto number_of_idle_thread = get_next_idle_thread();
    // now get Thread instance of this thread
    Thread &thread = _threads[number_of_idle_thread];
    // now send the function pointer "function_to_execute"
    // to the thread via its queue. The thread will execute it
    thread.queue.enqueue(function_to_execute);
    // returns a future to the promise the caller
}

int ThreadPool::get_next_idle_thread() { return _idle_threads_queue.dequeue(); }

void ThreadPool::notify_being_idle(const std::uint16_t thread_number) { _idle_threads_queue.enqueue(thread_number); }

void *ThreadPool::thread_function(ThreadPool *tp) {
    // first copy the content of the passed ThreadArguments
    uint16_t thread_number = 0;
    {
        pthread::unique_lock<pthread::mutex> guard(tp->_thread_args_copied_mutex);
        thread_number = tp->_thread_number;
        tp->_thread_args_copied.notify_one(); // signals that the _thread_number member of the ThreadPool can now be reused to
		                                      // start the next thread
    }
    

    // now get the queue for receiving commands
    auto &queue = tp->_threads[thread_number].queue;
    while (true) {
        tp->notify_being_idle(thread_number);        // signal to be idle and ready for a new task
        auto function_to_execute = queue.dequeue();  // wait for new function to execute
        // a nullptr is a request for ending the thread
        if (function_to_execute == nullptr) {
            return nullptr;
        }
        // execute the received function
        try {
            // now execute the function
            function_to_execute(thread_number);
        } catch (const std::exception &exc) {
            std::ostringstream ss;
            ss << "(" << thread_number << ") exception thrown: " << exc.what() << std::endl;
            tcerr << ss.str();
        } catch (...) {
            std::ostringstream ss;
            ss << "(" << thread_number << ") unknown exception thrown" << std::endl;
            tcerr << ss.str();
        }
    }
    return nullptr;
}

