#include "thread.h"

// the id of the thread creating an instance of "thread" is used as invalid thread id
// since a successfully created thread always have an id differing from the that of the creating thead
std::thread::thread() noexcept : _invalid_pthread(pthread_self()), _thread(pthread_self()) {}

std::thread::thread(void *(*thread_function)(void *), void *arg_ptr) : _invalid_pthread(pthread_self()) {
    pthread_create(&_thread, nullptr, thread_function, arg_ptr);
}

void std::thread::join() { pthread_join(_thread, nullptr); }

std::thread::~thread() {
    if (!pthread_equal(_thread, _invalid_pthread)) {
        pthread_cancel(_thread);
    }
}

#if defined(_WIN32)
#include <windows.h>
unsigned int std::thread::hardware_concurrency() {
    SYSTEM_INFO info = {{0}};
    GetSystemInfo(&info);
    return info.dwNumberOfProcessors;
}

#else

// "stolen" from boost implementation
unsigned int std::thread::hardware_concurrency() {
#if defined(PTW32_VERSION) || defined(__hpux)
    return pthread_num_processors_np();
#elif defined(__APPLE__) || defined(__FreeBSD__)
    int count;
    size_t size = sizeof(count);
    return sysctlbyname("hw.ncpu", &count, &size, NULL, 0) ? 0 : count;
#elif defined(BOOST_HAS_UNISTD_H) && defined(_SC_NPROCESSORS_ONLN)
    int const count = sysconf(_SC_NPROCESSORS_ONLN);
    return (count > 0) ? count : 0;
#elif defined(_GNU_SOURCE)
    return get_nprocs();
#else
    return 0;
#endif
}

#endif  // WINVER