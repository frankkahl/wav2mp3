#include "return_code.h"

#include "thread_includes.h"
using namespace std;

static int return_code = RET_CODE_OK;
static pthread::mutex return_code_mutex;

int get_return_code() {
    pthread::lock_guard<pthread::mutex> lock(return_code_mutex);
    return return_code;
}

void set_return_code(int ret_code) {
    pthread::lock_guard<pthread::mutex> lock(return_code_mutex);
    if (ret_code > return_code) {
        return_code = ret_code;
    }
}
