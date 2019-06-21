#include "tiostream.h"

std::mutex tostream::_mutex;

tostream::tostream(std::ostream &stream) : _stream(stream) {}

tostream tcout(std::cout);
tostream tcerr(std::cerr);