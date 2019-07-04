#include "tiostream.h"

pthread::mutex tostream::_mutex;

tostream::tostream(std::ostream &stream) : _stream(stream) {}

// instances of thread safe std::ostream derived classes encapsulating cout and cerr
tostream tcout(std::cout);
tostream tcerr(std::cerr);