#ifndef TOILET_PARSER_H_
#define TOILET_PARSER_H_

#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

#include "debug.hpp"

#include "common.hpp"
#include "errors.hpp"
#include "format.hpp"
#include "types.hpp"

namespace toiletdb {

class InMemoryFileParser
{
private:
    const std::string filename;
    size_t format_version;
    TableInfo columns;

    std::fstream open(const std::ios_base::openmode mode);
    void update_version(std::fstream &file);
    void read_types(std::fstream &file);
    std::vector<std::shared_ptr<ColumnBase>> deserealize(std::fstream &file, std::vector<std::string> &names);
    void serialize(std::fstream &file, const std::vector<std::shared_ptr<ColumnBase>> &columns);

public:
    InMemoryFileParser(const std::string filename);
    ~InMemoryFileParser();
    const size_t &get_version() const;
    const size_t &id_column_index() const;
    bool exists() const;
    bool exists_or_create() const;
    std::vector<std::shared_ptr<ColumnBase>> read_file();
    void write_file(const std::vector<std::shared_ptr<ColumnBase>> &columns);
    const std::vector<int> &types() const;
    const std::vector<std::string> &names() const;
};

} // namespace toiletdb

#endif // TOILET_IN_MEMORY_PARSER_H_
