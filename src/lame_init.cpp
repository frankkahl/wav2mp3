#include "lame_init.h"
#include "tiostream.h"
#include "return_code.h"

#include <lame/lame.h>
#include <stdio.h>
#include <functional>
#include <map>
#include <memory>
#include <sstream>
using namespace std;

void LameInit::lame_set_error_handler() {
    int res = 0;
    res = lame_set_errorf(lgf, discard_lame_output);
    check_error(res, "lame_set_errorf");

    res = lame_set_debugf(lgf, discard_lame_output);
    check_error(res, "lame_set_errorf");

    res = lame_set_msgf(lgf, discard_lame_output);
    check_error(res, "lame_set_errorf");
}

void LameInit::discard_lame_output(const char *format, va_list ap) { return; }

LameInit::LameInit() : lgf(lame_init()) {
    if (lgf) {
        lame_set_error_handler();
    }
}

LameInit::~LameInit() {
    if (lgf) {
        int res = lame_close(lgf);
        check_error(res, "lame_close", false);
    }
}

bool LameInit::is_initialized() const { return lgf != 0;} 
LameInit::operator lame_global_flags *() const { return lgf; }

void LameInit::check_error(int errnum, const string lame_function_name,
                           bool throw_exception) {
    // all error codes of lame are < 0 since returned values > 0 usually
    // mean e.g. the number of converted bytes
    if (errnum < 0) {
        ostringstream ss;
        ss << "LAME function \"" << lame_function_name;
        if (lame_error_map.count(errnum)) {
            ss << "\" failed with error: " << lame_error_map[errnum] << " (" << errnum << ")" << endl;
        } else {
            ss << "\" failed with unknown error code: " << errnum << endl;        
        }
        set_return_code(RET_CODE_LAME_ERROR);
        if (throw_exception) {
            throw lame_exception(ss.str());
        } else {
            tcerr << ss.str();
        }
    }
}

map<int, string> LameInit::lame_error_map = {
    {LAME_NOERROR, "LAME_NOERROR"},
    {LAME_GENERICERROR, "LAME_GENERICERROR"},
    {LAME_NOMEM, "LAME_NOMEM"},
    {LAME_BADBITRATE, "LAME_BADBITRATE"},
    {LAME_BADBITRATE, "LAME_BADBITRATE"},
    {LAME_BADSAMPFREQ, "LAME_BADSAMPFREQ"},
    {LAME_INTERNALERROR, "LAME_INTERNALERROR"},
    {FRONTEND_READERROR, "FRONTEND_READERROR"},
    {FRONTEND_WRITEERROR, "FRONTEND_WRITEERROR"},
    {FRONTEND_FILETOOLARGE, "FRONTEND_FILETOOLARGE"}
};