#pragma once

#include <iostream>
#include <vector>

#include "student.cpp"

template <typename T, typename A>
void debug_putv(std::vector<T, A> v, const char *name)
{
    std::cout << "*** " << name << ": [\n";
    for (T s : v) {
        std::cout << "\t" << s << ",\n";
    }
    std::cout << "]\n";
};

template <typename T> void debug_puts(T s, const char *name)
{
    std::cout << "*** " << name << ": " << s << "\n";
};
