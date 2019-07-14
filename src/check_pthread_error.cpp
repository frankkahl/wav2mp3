#include "check_pthread_error.h"
#include "return_code.h"
#include <cstring>
#include <sstream>
#include <iostream>

using namespace std;

// under Linux the dreaded _GNU_SOURCE is defined by default
// and insanely enough also required by the libstd++ header to work properly
// This means that according to the documentation I cannot use the POSIX compliant version of strerror_r

#if defined(_WIN32)
#  define strerror_r(errno, buf, len) strerror_s(buf, len, errno) 
#endif

#define MAX_ERROR_STRING_LENGTH 256
void check_pthread_error(int errnum, const string &pthread_function_name) {
    if (errnum) {
        ostringstream ss;
        char errmsg[MAX_ERROR_STRING_LENGTH];
	// it turns out that under Ubuntu Linux using g++ 9.1
	// the GNU version of strerror_r is used by default
	// which returns a pointer to the error string
#if defined(_WIN32)
	strerror_s(errmsg, MAX_ERROR_STRING_LENGTH, errnum);
	char *msg = errmsg;
#elif defined(_GNU_SOURCE)
        char * msg = strerror_r(errnum, errmsg, MAX_ERROR_STRING_LENGTH);
#else
	char *msg = "<unsupported platform: cannot retrieve error string>";
#endif
        ss << "Pthread function \"" << pthread_function_name;
        ss << "\" failed with error: " << string(msg) << " (" << errnum << ")" << endl;
        cout << ss.str();
        exit(RET_CODE_PTHREAD_ERROR);
    }
}
