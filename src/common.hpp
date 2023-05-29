#ifndef TOILET_COMMON_H_
#define TOILET_COMMON_H_

#include <cctype>
#include <climits>
#include <stdexcept>
#include <string>

#define TOILETDB_VERSION "1.0.5"

#define TDB_NOT_FOUND (size_t)(-1)

#define TDB_INVALID_ULL (size_t)(-1)
#define TDB_INVALID_I 2147483647

namespace toiletdb {

/// @brief Parse unsigned long long.
/// @return TDB_INVALID_ULL if string cannot be parsed.
size_t parse_long_long(const std::string &str);
/// @brief Parse signed int.
/// @return TDB_INVALID_I if string cannot be parsed.
int parse_int(const std::string &str);
/// @brief Returns copy of a string with all characters lowercased.
std::string to_lower_string(const std::string &str);
/// @brief Replaces all characters in a string with their lowercase variants.
void to_lower_pstring(std::string &str);

} // namespace toiletdb

#endif // TOILET_COMMON_H_
