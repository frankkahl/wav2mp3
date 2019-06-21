#ifndef THREADPOOLIMPL_H
#define THREADPOOLIMPL_H

#include <cstdint>
#include <future>
#include <iostream>
#include <sstream>
#include "signal_handler.h"
#include "thread_pool.h"
#include "tiostream.h"

template <typename T>
ThreadPool<T>::ThreadPool(const std::uint16_t num_of_threads) : _threads(num_of_threads) {
    if (num_of_threads < 1) {
        std::ostringstream ss;
        ss << "num_of_threads (" << num_of_threads << ") must not be smaller than 1";
        throw std::invalid_argument(ss.str());
    }
    start_all_threads();
    std::cout << "Thread pool started with " << num_of_threads << " threads." << std::endl;
};

template <typename T>
ThreadPool<T>::~ThreadPool() {
    stop_all_threads();
    std::cout << "Thread pool stopped." << std::endl;
};

template <typename T>
void ThreadPool<T>::start_all_threads() {
    for (std::uint16_t i = 0; i < _threads.size(); ++i) {
        _threads[i].thread.reset(new std::thread(&ThreadPool<T>::thread_method, this, i));
    }
}

template <typename T>
void ThreadPool<T>::stop_all_threads() {
    // notify all threads to terminate by sending a nullptr
    for (auto &thread : _threads) {
        thread.queue.enqueue(nullptr);
    }
    // then join the threads. The join() method waits until the thread has terminated
    for (auto &thread : _threads) {
        thread.thread->join();
    }
}

template <typename T>
std::future<T> ThreadPool<T>::enqueue(std::function<T(const std::uint16_t)> function_to_execute) {
    // get the number of a thread which is idle
    // wait if none is idle yet
    auto number_of_idle_thread = get_next_idle_thread();
    // now get Thread instance of this thread
    Thread &thread = _threads[number_of_idle_thread];
    // and store a promise instance in it which the thread uses to store
    // the result of the function "function_to_execute"
    thread.promise = std::promise<T>();
    // now send the function pointer "function_to_execute"
    // to the thread via its queue. The thread will execute it
    // and store its result into the promise
    thread.queue.enqueue(function_to_execute);
    // returns a future to the promise the caller
    return _threads[number_of_idle_thread].promise.get_future();
}

template <typename T>
int ThreadPool<T>::get_next_idle_thread() {
    return _idle_threads_queue.dequeue();
}

template <typename T>
void ThreadPool<T>::notify_being_idle(const std::uint16_t thread_number) {
    _idle_threads_queue.enqueue(thread_number);
}

template <typename T>
void ThreadPool<T>::thread_method(const std::uint16_t thread_number) {
    while (true) {
        notify_being_idle(thread_number);
        auto function_to_execute = _threads[thread_number].queue.dequeue();
        if (function_to_execute == nullptr) {
            return;
        }
        // execute the received function and store its result or the thrown exception
        // into the promise
        try {
            // now execute the function
            T res = function_to_execute(thread_number);
            // if this point is reached no exception has been thrown
            _threads[thread_number].promise.set_value(res);
        } catch (...) {
            std::ostringstream ss;
            ss << "(" << thread_number << ") exception thrown" << std::endl;
            tcerr << ss.str();
            auto exc = std::current_exception();
            try {
                _threads[thread_number].promise.set_exception(exc);
            } catch (...) {  // just make sure that the thread is not terminated by an escaping exception
            }
        }
    }
}

#endif  // THREADPOOLIMPL_H