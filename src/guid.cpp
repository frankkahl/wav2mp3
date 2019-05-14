#include "guid.h"
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <iomanip>
#include <ios>

using namespace std;

// private static regular expression describing the valid human readable GUID string
// of format
//     XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
// where X stands for a hexadecimal digit: 0-9, a-f, A-F
regex Guid::guid_regexp = std::regex(R"([a-f0-9]{8}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{4}-[a-f0-9]{12})", std::regex::icase);
// static member representing the empty GUID for which all members are zero
Guid Guid::empty;

Guid::Guid(const std::string &str) {
    if (regex_match(str, guid_regexp)) {
        time_low = stoul(str.substr(0, 8), nullptr, 16);
        time_mid = (uint16_t)stoul(str.substr(9, 4), nullptr, 16);  // for some reason there is no stou function
        time_hi_and_version = (uint16_t)stoul(str.substr(14, 4), nullptr, 16);
        clock_seq_low = (uint8_t)stoul(str.substr(19, 2), nullptr, 16);
        clock_seq_hi = (uint8_t)stoul(str.substr(21, 2), nullptr, 16);
        for (int i = 0; i < 6; i ++) {
            node[i] = (uint8_t)stoul(str.substr(24+2*i, 2), nullptr, 16);
        }
    }
}

Guid::Guid() {}

// returns guid as a human readable string
// of format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
// where X stands for a hexadecimal digit: 0-9, a-f
string Guid::string() const {
    ostringstream ss;
    // the casting of the uint8_t values to (uint16_t) is required before output since otherwise
    // they will be interpreted as character
    ss << right << setfill('0') << setw(8) << hex << time_low << '-' << setw(4) << time_mid << '-' << setw(4) << time_hi_and_version << '-' 
       << setw(2) << (uint16_t)clock_seq_low << setw(2) << (uint16_t)clock_seq_hi << '-';
    for (int i = 0; i < 6; i++) {
        ss << setw(2) << (uint16_t)node[i];
    }
    return ss.str();
}

bool Guid::is_empty() const { return *this == empty; }

// some related free functions:
// comparison and stream output

bool operator==(const Guid &a, const Guid &b) {
    return (a.time_low == b.time_low)
        && (a.time_mid == b.time_mid)
        && (a.time_hi_and_version == b.time_hi_and_version)
        && (a.clock_seq_low == b.clock_seq_low)
        && (a.clock_seq_hi == b.clock_seq_hi)
        && (memcmp(a.node, b.node, sizeof(a.node)) == 0);
}

bool operator!=(const Guid &a, const Guid &b) { return (!operator==(a, b)); }

bool operator<(const Guid &a, const Guid &b) { return a.string() < b.string(); }

bool operator<=(const Guid &a, const Guid &b) { return a.string() <= b.string(); }

bool operator>=(const Guid &a, const Guid &b) { return b <= a; }

bool operator>(const Guid &a, const Guid &b) { return b < a; }

std::ostream &operator<<(std::ostream &out, const Guid &guid) {
    out << guid.string();
    return out;
}
