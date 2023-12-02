#ifndef TDB_DEBUG_H
#define TDB_DEBUG_H

#ifndef NDEBUG
    #include <cassert>
    #include <iostream>
    #include <vector>

    #define TDB_ASSERT(boolval) assert(boolval)
    #define TDB_TRACE_VECTOR(vector, name) toiletdb::debug::trace(vector, name)
    #define TDB_TRACE(value, name) toiletdb::debug::trace(value, name)
#else
    #define TDB_ASSERT(boolval)
    #define TDB_TRACE_VECTOR(vector, name)
    #define TDB_TRACE(value, name)
#endif

namespace toiletdb {
namespace debug {

#ifndef NDEBUG

template <typename T, typename A>
void trace_vector(const std::vector<T, A> &v, const char *name)
{
    std::cout << "[TRACE] vector " << name << ": [\n";
    for (const T &s : v) {
        std::cout << "\t'" << s << "',\n";
    }
    std::cout << "]\n";
    fflush(stdout);
};

template <typename T>
void trace(const T &s, const char *name)
{
    std::cout << "[TRACE] " << name << ": '" << s << "'\n";
    fflush(stdout);
}

#endif // NDEBUG

} // namespace debug
} // namespace toiletdb

#endif // TDB_DEBUG_H
