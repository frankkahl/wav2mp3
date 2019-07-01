#ifndef TIOSTREAM_H
#define TIOSTREAM_H

#include "thread_includes.h"
#include <iostream>
#include <string>

// wrapper class to make streams inherited from std::ostream
// thread safe

class tostream {
   public:
    tostream(std::ostream &stream);
    template <typename T>
    tostream &operator<<(const T &out) {
        std::lock_guard<std::mutex> guard(_mutex);
        _stream << out;
        return *this;
    }

   private:
    static std::mutex _mutex;
    std::ostream &_stream;
};

// declare thread safe instances encapsulating cout and cerr instatiated in tiostream.cpp
extern tostream tcout;
extern tostream tcerr;

#endif // TIOSTREAM_H
