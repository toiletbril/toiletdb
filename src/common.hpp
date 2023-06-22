#ifndef TOILET_COMMON_H_
#define TOILET_COMMON_H_

#include <cctype>
#include <climits>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#define TOILETDB_VERSION "1.3.0"

#define TDB_NOT_FOUND (size_t)(-1)

#define TDB_INVALID_ULL (size_t)(-1)
#define TDB_INVALID_I 2147483647

namespace toiletdb {
/**
 *  @brief Parse size_t.
 *  @return TDB_INVALID_ULL if string cannot be parsed.i
 */
size_t parse_long_long(const std::string &str);
/**
 *  @brief Parse signed int.
 *  return TDB_INVALID_I if string cannot be parsed.
 */
int parse_int(const std::string &str);
/**
 *  @brief Returns copy of a string with all characters lowercased.
 */
std::string to_lower_string(const std::string &str);
/**
 *  @brief Replaces all characters in a string with their lowercase variants.
 */
void to_lower_pstring(std::string &str);

template <class T>
std::vector<std::shared_ptr<T>> vector_raw_into_shared(std::vector<T *> &v)
{
    std::vector<std::shared_ptr<T>> result;
    result.reserve(v.size());

    for (T *e : v)
        result.push_back(std::make_shared<T>(e));

    return result;
}

template <class T>
std::vector<T *> vector_shared_into_raw(std::vector<std::shared_ptr<T>> &v)
{
    std::vector<T *> result;
    result.reserve(v.size());

    for (std::shared_ptr<T> e : v)
        result.push_back(e.get());

    return result;
}

} // namespace toiletdb

#endif // TOILET_COMMON_H_
