#include "debug.hpp"

namespace toiletdb {

#ifndef NDEBUG
template <typename T, typename A>
void toilet_debug_putv(const std::vector<T, A> &v, const char *name);

template <typename T> void toilet_debug_puts(const T &s, const char *name);
#endif

} // namespace toiletdb
