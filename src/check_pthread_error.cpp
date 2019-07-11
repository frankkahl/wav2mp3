#define __STDC_WANT_LIB_EXT1__ 1
#include "check_pthread_error.h"
#include "return_code.h"
#include <cstring>
#include <sstream>
#include <iostream>

using namespace std;

#define MAX_ERROR_STRING_LENGTH 256
void check_pthread_error(int errnum, const string pthread_function_name) {
    if (errnum) {
        ostringstream ss;
        char errmsg[MAX_ERROR_STRING_LENGTH + 1];
        strerror_s(errmsg, MAX_ERROR_STRING_LENGTH, errnum);
        ss << "Pthread function \"" << pthread_function_name;
        ss << "\" failed with error: " << string(errmsg) << " (" << errnum << ")" << endl;
        cout << ss.str();
        exit(RET_CODE_PTHREAD_ERROR);
    }
}
