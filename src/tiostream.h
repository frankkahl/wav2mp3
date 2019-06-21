#ifndef TIOSTREAM_H
#define TIOSTREAM_H

#include <iostream>
#include <string>
#include <mutex>

class tostream {
   public:
    tostream(std::ostream &stream);
    template <typename T>
    tostream &operator<<(const T &out) {
        std::lock_guard guard(_mutex);
        _stream << out;
        return *this;
    }

   private:
    static std::mutex _mutex;
    std::ostream &_stream;
};

extern tostream tcout;
extern tostream tcerr;

#endif // TIOSTREAM_H
