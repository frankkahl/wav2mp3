#ifndef LAME_WRAPPER_H
#define LAME_WRAPPER_H

#include <string>
#include "lame.h"

class LameInit {
   public:
    LameInit();
    ~LameInit();
    bool is_initialized() const;
    operator lame_global_flags *() const;
    // string lame_errors() const;
   private:  // private methods
             // void lame_set_error_handler();
             // void report_function(const char *format, va_list ap);
   private:  // private members
    lame_global_flags *lgf;
    // ostringstream lame_errors;
};

#endif  // LAME_WRAPPER_H
