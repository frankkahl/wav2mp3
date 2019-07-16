#ifndef THREAD_INCLUDES_H
#define THREAD_INCLUDES_H

#ifdef USE_CPP11_THREADS
#include <condition_variable>
#include <mutex>
#include <thread>
namespace pthread = std;
#else
#include "condition_variable.h"
#include "mutex.h"
#include "thread.h"
#endif

#endif  // THREAD_INCLUDES_H
