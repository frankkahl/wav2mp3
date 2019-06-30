#ifndef THREAD_INCLUDES_H
#define THREAD_INCLUDES_H

#ifdef USE_PTHREAD
#include "condition_variable.h"
#include "mutex.h"
#include "thread.h"
#else
#include <condition_variable>
#include <mutex>
#include <thread>
#endif

#endif  // THREAD_INCLUDES_H