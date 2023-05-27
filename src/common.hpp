#ifndef TOILET_COMMON_H_
#define TOILET_COMMON_H_

#include <cctype>
#include <climits>
#include <stdexcept>
#include <string>

#define TOILETDB_VERSION "1.0.1"

#define TDB_NOT_FOUND (size_t)(-1)

#define TDB_INVALID_ULL (size_t)(-1)
#define TDB_INVALID_I 2147483647

namespace toiletdb {

/// @brief Parse unsigned long long.
/// @return TDB_INVALID_ULL if string cannot be parsed.
size_t cm_parsell(std::string &str);

/// @brief Parse signed int.
/// @return TDB_INVALID_I if string cannot be parsed.
int cm_parsei(std::string &str);

/// @brief Returns copy of a string with all characters lowercased.
std::string cm_str_tolower(std::string &str);

/// @brief Replaces all characters in a string with their lowercase variants.
void cm_pstr_tolower(std::string &str);

} // namespace toiletdb

#endif // TOILET_DEBUG_H_