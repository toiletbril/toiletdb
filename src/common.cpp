#pragma once

#include <string>

#define COMMON_INVALID_NUMBERLL (size_t)(-1)

size_t cm_parsell(std::string &str)
{
    if (str == "0") {
        return 0;
    } else {
        size_t n = std::atoll(str.c_str());
        if (n == 0)
            return COMMON_INVALID_NUMBERLL;
        return n;
    }
}
