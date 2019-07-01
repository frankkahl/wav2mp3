#include "return_code.h"

#include "thread_includes.h"
using namespace std;

static int return_code = RET_CODE_OK;
static std::mutex return_code_mutex;

int get_return_code() {
    lock_guard<mutex> lock(return_code_mutex);
    return return_code;
}

void set_return_code(int ret_code) {
    lock_guard<mutex> lock(return_code_mutex);
    if (ret_code > return_code) {
        return_code = ret_code;
    }
}
