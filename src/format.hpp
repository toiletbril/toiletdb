#ifndef TOILET_FORMAT_H_
#define TOILET_FORMAT_H_

#include <memory>
#include <vector>

#include "debug.hpp"

#include "errors.hpp"
#include "types.hpp"

namespace toiletdb {

struct FormatOne
{
    static size_t read_version(std::fstream &file);
    static TableInfo read_types(std::fstream &file);
    static std::vector<std::shared_ptr<ColumnBase>> deserealize(std::fstream &file,
                                                 TableInfo &columns,
                                                 std::vector<std::string> &names);
    static void write_header(std::fstream &file,
                             const std::vector<std::shared_ptr<ColumnBase>> &data);
    static void serialize(std::fstream &file,
                          const std::vector<std::shared_ptr<ColumnBase>> &data);
};

} // namespace toiletdb

#endif // TOILET_FORMAT_ONE_H_
