#ifndef LAME_INIT_H
#define LAME_INIT_H

#include <lame/lame.h>
#include <string>
#include <map>
#include <stdexcept>

// exception thrown by LameInit::check_error(...)
class lame_exception : public std::runtime_error {
   public:
    lame_exception(const std::string &message) : std::runtime_error(message) {}
    lame_exception(const char *message) : std::runtime_error(message) {}
};

// RAII style resource manager for lame library

class LameInit {
   public:
    LameInit();
    ~LameInit();
    bool is_initialized() const;
    // allows to use an instance of LameInit instead of a
    // lame_global_flags pointer
    operator lame_global_flags *() const;
    // checks if errnum is a valid lame error (means: < 0).
    // If yes then either print an error message to tcerr if throw_exception == false
    // or throws a lame_exception with the error message
    static void check_error(int errnum, const std::string lame_function_name, bool throw_exception = true);

    // maps lame error number to descriptive string
    static std::map<int, std::string> lame_error_map;

    // private methods
   private:
    // registers discard_lame_output static function as lame error/message output
    // function
    void lame_set_error_handler();
    static void discard_lame_output(const char *format, va_list ap);

   private:  // private members
    lame_global_flags *lgf;
};

#endif  // LAME_INIT_H
