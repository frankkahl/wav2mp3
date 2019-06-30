#ifndef SIGNAL_HANDLER
#define SIGNAL_HANDLER

#include "thread_includes.h"

class SignalHandler {
   public:
    SignalHandler();
    virtual ~SignalHandler();

    static bool termination_requested();
   private:
    static bool _termination_requested;
    static std::mutex _mutex;
    static void signal_handler(int signal_code);
};

#endif // SIGNAL_HANDLER
