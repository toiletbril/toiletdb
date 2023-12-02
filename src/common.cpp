#include "common.hpp"

#include <optional>
#include <string>
#include <vector>

#include <sys/stat.h>

#ifdef _WIN32
    #define stat _stat
#endif

namespace toiletdb {

std::optional<size_t> parse_size_t(std::string_view str)
{
    size_t result = 0;
    for (const char &c : str) {
        if (std::isdigit(c)) result = result * 10 + (c - '0');
        else return std::nullopt;
    }
    return std::optional<size_t>(result);
}

std::optional<int> parse_signed_int(std::string_view str)
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
            return std::nullopt;
        }
    }

    return std::optional<int>(result * mult);
}

std::string to_lowercase(std::string_view str)
{
    std::string result;

    for (const char &c : str) {
        result += std::tolower(c);
    }

    return result;
}

void to_lowercase_in_place(std::string &str)
{
    for (char &c : str) {
        c = std::tolower(c);
    }
}

bool file_exists(std::string_view file_path)
{
    struct stat buffer;
    return (stat(file_path.data(), &buffer) == 0);
}

} // namespace toiletdb
