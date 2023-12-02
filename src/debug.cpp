#include "debug.hpp"

namespace toiletdb {

#ifndef NDEBUG

template <typename T, typename A>
void trace_vector(const std::vector<T, A> &v, const char *name);

template <typename T>
void trace(const T &s, const char *name);

#endif

} // namespace toiletdb
