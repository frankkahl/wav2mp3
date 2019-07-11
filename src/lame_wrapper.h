#ifndef LAME_WRAPPER_H
#define LAME_WRAPPER_H

#include <lame/lame.h>
#include <string>
#include <map>
#include <stdexcept>

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
    static void check_error(int errnum, const std::string lame_function_name, bool throw_exception = true);
    static std::map<int, std::string> lame_error_map;

    // private methods
   private:
    // registers print_lame_output static funciton as lame error/message output
    // function
    void lame_set_error_handler();
    // static void discard_lame_output(const char *format, va_list ap);
    static void print_lame_output(const char *format, va_list ap);

   private:  // private members
    lame_global_flags *lgf;
};

#endif  // LAME_WRAPPER_H
