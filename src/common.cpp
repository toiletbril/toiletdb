#include "common.hpp"

namespace toiletdb {

size_t parse_long_long(const std::string &str)
{
    size_t result = 0;

    for (const char &c : str) {
        if (std::isdigit(c)) {
            result = result * 10 + (c - '0');
        }
        else {
            return TDB_INVALID_ULL;
        }
    }

    return result;
}

int parse_int(const std::string &str)
{
    int result = 0;
    int mult   = 1;

    for (const char &c : str) {
        if (c == '-' && result == 0) {
            mult *= -1;
        }
        else if (std::isdigit(c)) {
            result = result * 10 + (c - '0');
        }
        else {
            return TDB_INVALID_I;
        }
    }

    return result * mult;
}

std::string to_lower_string(const std::string &str)
{
    std::string result;

    for (const char &c : str) {
        result += std::tolower(c);
    }

    return result;
}

// Replaces all characters in target string with lowercase ones.
void to_lower_pstring(std::string &str)
{
    for (char &c : str) {
        c = std::tolower(c);
    }
}

template <class T>
std::vector<std::shared_ptr<T>> vector_raw_into_shared(std::vector<T *> &v);

template <class T>
std::vector<T *> vector_shared_into_raw(std::vector<std::shared_ptr<T>> &v);

} // namespace toiletdb
