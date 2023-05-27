#ifndef TOILET_PARSER_H_
#define TOILET_PARSER_H_

#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#ifndef NDEBUG
    #include "debug.hpp"
#endif

#include "common.hpp"
#include "format.hpp"
#include "types.hpp"

namespace toiletdb {

class InMemoryFileParser
{
private:
    const char *const &filename;
    size_t format_version;
    TableInfo columns;

    struct Private;

public:
    InMemoryFileParser(const char *const &filename);

    ~InMemoryFileParser();

    size_t get_version() const;

    size_t get_id_column_index() const;

    bool exists() const;

    bool exists_or_create() const;

    std::vector<Column *> read_file();

    void write_file(const std::vector<Column *> &columns) const;
};

} // namespace toiletdb

#endif // TOILET_IN_MEMORY_PARSER_H_