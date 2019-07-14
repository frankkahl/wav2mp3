#ifndef RETURN_CODE_H
#define RETURN_CODE_H

// known return codes
// they larger the numerical value of the return code is
// the more severe the error is
#define RET_CODE_OK 0
#define RET_CODE_CONVERTING_SOME_FILES_FAILED 1
#define RET_ABORTED_BY_SIGINT_OR_SIGTERM 2
#define RET_CODE_DIR_ITER_FAILED 3
#define RET_CODE_EXCEPTION_CAUGHT 4
#define RET_CODE_PTHREAD_ERROR 5
#define RET_CODE_LAME_ERROR 6

// gets the return code for the whole program in a thread safe way
extern int get_return_code();
// sets the return code for the whole program
// but only if the return code is numerically larger than the already set one
// The reason is that the returns are sorted by severity in ascending order
// This function is thread safe
extern void set_return_code(int ret_code);

#endif  // RETURN_CODE_H
