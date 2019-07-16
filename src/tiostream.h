#ifndef TIOSTREAM_H
#define TIOSTREAM_H

#include <iostream>
#include <string>
#include "thread_includes.h"

// wrapper class to make streams inherited from std::ostream
// thread safe

class tostream {
  public:
    tostream(std::ostream &stream);
    template <typename T>
    tostream &operator<<(const T &out) {
        pthread::lock_guard<pthread::mutex> guard(_mutex);
        _stream << out;
        return *this;
    }

  private:
    static pthread::mutex _mutex;
    std::ostream &        _stream;
};

// declare thread safe instances encapsulating cout and cerr instatiated in tiostream.cpp
extern tostream tcout;
extern tostream tcerr;

#endif  // TIOSTREAM_H
