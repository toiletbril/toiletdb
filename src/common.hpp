#ifndef TDB_COMMON_H
#define TDB_COMMON_H

#include <optional>
#include <string>
#include <vector>

#define TDB_MAJOR_VERSION 2
#define TDB_MINOR_VERSION 0
#define TDB_PATCH_VERSION 0

namespace toiletdb {

std::optional<size_t> parse_size_t(std::string_view str);
std::optional<int> parse_signed_int(std::string_view str);
std::string to_lowercase(std::string_view str);
void to_lowercase_in_place(std::string str);
bool file_exists(std::string_view file_path);

} // namespace toiletdb

#endif // TDB_COMMON_H
