#ifndef TOILET_IN_MEMORY_TABLE_H_
#define TOILET_IN_MEMORY_TABLE_H_

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numeric>
#include <vector>

#ifndef NDEBUG
    #include "debug.hpp"
#endif

#include "common.hpp"
#include "parser.hpp"

namespace toiletdb {

/**
 * @class InMemoryTable
 * @brief Medium level abstraction representing one table.
 * Each table represents one file.
 */
class InMemoryTable
{
private:
    std::vector<unsigned long long> index;
    std::vector<Column *> columns;
    InMemoryFileParser *parser;

    struct Private;

public:
    /// @brief Opens up a file and loads up into memory.
    /// @warning Does not create a file. Will throw an error.
    InMemoryTable(const char *const &filename);

    ~InMemoryTable();

    const std::vector<Column *> &get_all() const;

    /// @brief Discards all changes made to in-memory vector, and reads file
    /// again.
    void reread_file();

    /// @brief Writes data stored in memory back to the file.
    void write_file() const;

    /// @brief Search in-memory vector by ID.
    /// O(log n)
    /// @return TDB_NOT_FOUND if element is not found.
    size_t search(const size_t &id) const;

    /// @brief Search in-memory vector by comparing values as std::string.
    /// O(n)
    /// @return TDB_NOT_FOUND if element is not found.
    std::vector<size_t> search(const std::string &name,
                               std::string &query) const;

    /// @brief Get one row from vector.
    /// One row means a value from each column.
    /// @warning You will need to get types and cast them yourself.
    /// @see get_column_types()
    /// @see get_column_type()
    std::vector<void *> get_row(const size_t &pos);

    /// @brief Adds row from
    /// One row means a value from each column.
    /// @warning You will need to get types and cast them yourself.
    /// @see get_column_types()
    /// @see get_column_type()
    int add(std::vector<std::string> &args);

    /// @brief Erases element with ID.
    bool erase_id(const size_t &id);

    /// @brief Clears all columns.
    void clear();

    size_t get_column_count() const;

    const std::vector<std::string> get_column_names() const;

    /// @brief Searches for column with specified name in a table.
    /// @returns Position of column in the table. TDB_NOT_FOUND if element is
    /// not found.
    size_t search_column_index(const std::string &name) const;

    /// @see ToiletType
    const std::vector<int> get_column_types() const;

    size_t total_rows() const;

    size_t get_next_id() const;
};

} // namespace toiletdb

#endif // TOILET_IN_MEMORY_TABLE_H_