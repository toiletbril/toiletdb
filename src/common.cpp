#pragma once

#include <cctype>
#include <climits>
#include <string>

#define TOILET_VERSION "1.0.0"
#define TOILET_NAME "toiletdb"
#define TOILET_GITHUB "<https://github.com/toiletbril>"

#define COMMON_INVALID_NUMBERLL (size_t)(-1)
#define COMMON_INVALID_NUMBERI INT_MAX

size_t cm_parsell(std::string &str)
{
    size_t result = 0;

    for (char &c : str) {
        if (std::isdigit(c)) {
            result = result * 10 + (c - '0');
        }
        else {
            return COMMON_INVALID_NUMBERLL;
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
            return COMMON_INVALID_NUMBERI;
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
