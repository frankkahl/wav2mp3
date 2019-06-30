#include "thread.h"

std::thread::thread() noexcept : _invalid_pthread(pthread_self()), _thread(pthread_self()) {}

void std::thread::join() { pthread_join(_thread, nullptr); }

std::thread::~thread() {
    if (!pthread_equal(_thread, _invalid_pthread)) {
        pthread_cancel(_thread);
    }
}

#if defined(_WIN32)
#include <windows.h>
unsigned int std::thread::hardware_concurrency() {
#define _WIN32_WINNT _WIN32_WINNT_WINXP
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

#endif // WINVER