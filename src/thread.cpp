#include "thread.h"
#include "check_pthread_error.h"

// the id of the thread creating an instance of "thread" is used as invalid thread id
// since a successfully created thread always have an id differing from the that of the creating thead
pthread::thread::thread() noexcept
    : _invalid_pthread(pthread_self())
    , _thread(pthread_self()) {
}

pthread::thread::thread(void *(*thread_function)(void *), void *arg_ptr)
    : _invalid_pthread(pthread_self()) {
    int res = pthread_create(&_thread, nullptr, thread_function, arg_ptr);
    check_pthread_error(res, "pthread_create");
}

void pthread::thread::join() {
    pthread_join(_thread, nullptr);
}

pthread::thread::~thread() {
    if (!pthread_equal(_thread, _invalid_pthread)) {
        int res = pthread_cancel(_thread);
        if (res != ESRCH) {
            check_pthread_error(res, "pthread_cancel");
        }
    }
}

#if defined(_WIN32)
#include <windows.h>
unsigned int pthread::thread::hardware_concurrency() {
    SYSTEM_INFO info = {{0}};
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
}

#else

#include <sys/sysinfo.h>
// "stolen" from boost implementation
unsigned int pthread::thread::hardware_concurrency() {
#if defined(_GNU_SOURCE)
    return get_nprocs();
#else
    return 0;
#endif
}

#endif  // WINVER
