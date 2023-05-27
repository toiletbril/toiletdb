#ifndef TOILET_DEBUG_H_
#define TOILET_DEBUG_H_

#include <iostream>
#include <vector>

#ifndef NDEBUG
    #define TOILET_DEBUGV(v, name) toilet_debug_putv(v, name)
    #define TOILET_DEBUGS(s, name) toilet_debug_puts(s, name)
#else
    #define TOILET_DEBUGV(v, name)
    #define TOILET_DEBUGS(s, name)
#endif

namespace toiletdb {

#ifndef NDEBUG

template <typename T, typename A>
void toilet_debug_putv(const std::vector<T, A> &v, const char *name)
{
    std::cout << "*** " << name << ": [\n";
    for (const T &s : v) {
        std::cout << "\t'" << s << "',\n";
    }
    std::cout << "]\n";
    fflush(stdout);
};

template <typename T> void toilet_debug_puts(const T &s, const char *name)
{
    std::cout << "*** " << name << ": '" << s << "'\n";
    fflush(stdout);
}

#endif

} // namespace toiletdb

#endif // TOILET_DEBUG_H_