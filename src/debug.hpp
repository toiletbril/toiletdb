#ifndef TOILET_DEBUG_H_
#define TOILET_DEBUG_H_

#include <iostream>
#include <vector>

namespace toiletdb {

template <typename T, typename A>
void debug_putv(const std::vector<T, A> &v, const char *name);

template <typename T> void debug_puts(const T &s, const char *name);

} // namespace toiletdb

#endif // TOILET_DEBUG_H_