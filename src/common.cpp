#include "common.hpp"

namespace toiletdb {

size_t cm_parsell(std::string &str)
{
    size_t result = 0;

    for (char &c : str) {
        if (std::isdigit(c)) {
            result = result * 10 + (c - '0');
        }
        else {
            return TDB_INVALID_ULL;
        }
    }

    return result;
}

int cm_parsei(std::string &str)
{
    int result = 0;
    int mult   = 1;

    for (char &c : str) {
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

std::string cm_str_tolower(std::string &str)
{
    std::string result;
    for (char &c : str) {
        result += std::tolower(c);
    }
    return result;
}

// Replaces all characters in target string with lowercase ones.
void cm_pstr_tolower(std::string &str)
{
    for (char &c : str) {
        c = std::tolower(c);
    }
}

} // namespace toiletdb
