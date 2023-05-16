#pragma once

#include <string>
#include <cctype>

#define COMMON_INVALID_NUMBERLL (size_t)(-1)

size_t cm_parsell(std::string &str)
{
    size_t result = 0;

    for (char &c : str) {
        if (std::isdigit(c)) {
            result = result * 10 + (c - '0');
        } else {
            return COMMON_INVALID_NUMBERLL;
        }
    }

    return result;
}
