#ifndef TDB_TABLE_MANIP_H
#define TDB_TABLE_MANIP_H

#include <memory>
#include <vector>
#include <span>

#include "debug.hpp"

#include "errors.hpp"
#include "types.hpp"

namespace toiletdb {

size_t read_version_row(std::string_view line);
std::vector<Column_Info> read_types_row(std::string_view line);
std::vector<std::string> split_row(std::string_view line, size_t column_count);

std::string make_version_row(size_t version);
std::string make_types_row(std::span<const Column_Base *> columns);

} // namespace toiletdb

#endif // TDB_TABLE_MANIP_H
