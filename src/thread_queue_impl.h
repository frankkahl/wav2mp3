#include "thread_queue.h"

using namespace std;

template <typename T>
void ThreadQueue<T>::enqueue(T element) {
    pthread::unique_lock<pthread::mutex> lock(_mutex);
    _queue.push(element);
    _cond_var.notify_one();
}

template <typename T>
T ThreadQueue<T>::dequeue() {
    pthread::unique_lock<pthread::mutex> lock(_mutex);
    if (_queue.empty()) {
        _cond_var.wait(lock);
    }
    auto element = _queue.front();
    _queue.pop();
    return element;
}
