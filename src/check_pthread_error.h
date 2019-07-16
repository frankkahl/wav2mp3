#ifndef CHECK_PTHREAD_ERROR_H
#define CHECK_PTHREAD_ERROR_H

#include <string>

extern void check_pthread_error(int errnum, const std::string &pthread_function_name);

#endif  // CHECK_PTHREAD_ERROR_H
