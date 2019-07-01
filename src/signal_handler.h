#ifndef SIGNAL_HANDLER
#define SIGNAL_HANDLER

#include "thread_includes.h"

// class for capturing Ctrl-C (SITINT) and SIGTERM signals
// intended usage:
//    - constructing one and only one instace:
//      over the life span o the instance the static function signal_handler
//      is installed as handler for for the signals SIGINT (Ctrl-C) and SIGTERM
//    - static function termination_request() returns true if SIGINT and/or SIGTERM was received
//      and false otherwise
class SignalHandler {
   public:
    SignalHandler();
    virtual ~SignalHandler();

    // returns true if SIGINT and/or SIGTERM was received
    // and false otherwise
    static bool termination_requested();

   private:
    static bool _termination_requested;
    static std::mutex _mutex;
    static void signal_handler(int signal_code);
};

#endif  // SIGNAL_HANDLER
