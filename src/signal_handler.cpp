#include "signal_handler.h"
#include "tiostream.h"
#include "thread_includes.h"
#include "return_code.h"

#include <csignal>
#include <sstream>
using namespace std;

SignalHandler::SignalHandler() {
    signal(SIGINT, &SignalHandler::signal_handler);
    signal(SIGTERM, &SignalHandler::signal_handler);
}

SignalHandler::~SignalHandler() {
    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
}

bool SignalHandler::termination_requested() {
    lock_guard guard(SignalHandler::_mutex);
    return SignalHandler::_termination_requested;
}

void SignalHandler::signal_handler(int sig_number) {
    ostringstream ss;
    switch (sig_number) {
        case SIGINT:
            ss << "Ctrl-C pressed, aborting ..." << endl;
            tcout << ss.str();
            break;
        case SIGTERM:
            ss << "SIGTERM received, aborting ..." << endl;
            tcout << ss.str();
            break;
        default:
            return;
    }
    lock_guard guard(SignalHandler::_mutex);
    SignalHandler::_termination_requested = true;
    set_return_code(RET_ABORTED_BY_SIGINT_OR_SIGTERM);

}

bool SignalHandler::_termination_requested = false;
mutex SignalHandler::_mutex;