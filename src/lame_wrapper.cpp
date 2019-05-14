#include "lame_wrapper.h"

#include <stdio.h>
#include <functional>
#include <memory>
#include <sstream>
#include "lame.h"

using namespace std;

// void LameInit::lame_set_error_handler() {
//	auto handler = std::bind(&LameInit::report_function, this);
//	lame_set_errorf(lgf, reinterpret_pointer_cast<lame_report_function *>( handler));
//	lame_set_debugf(lgf, handler);
//	lame_set_msgf(lgf, handler);
//
//}

//// does not work yet
// void LameInit::report_function(const char *format, va_list ap) {
//	int string_size = snprintf(0, 0, format, ap);   // first calculate the size the error string
//													// will have
//	char *error_string = new char[string_size];
//	if (error_string) {
//		snprintf(error_string, string_size, format, ap);
//		lame_errors << error_string;
//		delete[] error_string;
//	}
//}

LameInit::LameInit() : lgf(lame_init()) {}

LameInit::~LameInit() {
    if (lgf) {
        lame_close(lgf);
    }
}

bool LameInit::is_initialized() const { return lgf != 0; }

LameInit::operator lame_global_flags*() const { return lgf; }