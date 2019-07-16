#ifndef GUID_H
#define GUID_H

#include <cstdint>
#include <regex>
#include <string>

// defines a Globally Unique IDentifier - sometimes also
// referred to as Universally Unique IDentifier
// following the layout given in the english Wikipedia:
// https://en.wikipedia.org/wiki/Universally_unique_identifier
typedef struct Guid {
    // data fields
    uint32_t time_low            = 0;
    uint16_t time_mid            = 0;
    uint16_t time_hi_and_version = 0;
    uint8_t  clock_seq_low       = 0;
    uint8_t  clock_seq_hi        = 0;
    uint8_t  node[6]             = {0};
    // methods
    // constructs GUID from the passed std::string guid_string
    // guid_string must be in the human readable format
    //     XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
    // where X stands for a hexadecimal digit: 0-9, a-f, A-F
    // if the construction
    Guid(const std::string &guid_string);
    Guid();  // default constructor setting all members to 0

    // returns guid as a human readable string
    // of format XXXXXXXX-XXXX-XXXX-XXXX-XXXXXXXXXXXX
    // where X stands for a hexadecimal digit: 0-9, a-f
    std::string string() const;
    // checks if the GUID is considered emtpy (all zero)
    bool is_empty() const;

    // some static data shared between all instances
    static std::regex guid_regexp;
    static Guid       empty;
} Guid;

extern bool operator==(const Guid &a, const Guid &b);
extern bool operator!=(const Guid &a, const Guid &b);
extern bool operator<(const Guid &a, const Guid &b);  // required for using Guid instances as std::map keys
extern bool operator<=(const Guid &a, const Guid &b);
extern bool operator>(const Guid &a, const Guid &b);  // required for using Guid instances as std::map keys
extern bool operator>=(const Guid &a, const Guid &b);

extern std::ostream &operator<<(std::ostream &out, const Guid &guid);

#endif  // GUID_H
