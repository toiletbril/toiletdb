#ifndef TOILET_IN_MEMORY_TABLE_H_
#define TOILET_IN_MEMORY_TABLE_H_

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <vector>

#include "debug.hpp"

#include "common.hpp"
#include "types.hpp"
#include "errors.hpp"
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
    struct Private;
    std::unique_ptr<Private> private_;

public:
    /// @brief Opens up a file and loads it up into memory.
    /// @warning Does not create a file. Will throw an error.
    /// @throws std::ios::failure when file cannot be opened.
    /// @throws std::runtime_error when file does not exist.
    /// @throws ParsingError when parsing error is encountered.
    InMemoryTable(const std::string &filename);
    ~InMemoryTable();
    const std::vector<Column *> &get_all() const;
    /// @brief Discards all changes made to in-memory vector, and reads file
    /// again.
    /// @throws std::runtime_error when table file was deleted or moved.
    void reread_file();
    /// @brief Writes data stored in memory back to the file.
    /// @throws std::runtime_error when table file was deleted or moved.
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
    ///        One row means a value from each column.
    /// @warning You will need to get types and cast them yourself.
    /// @see get_column_types()
    /// @see get_column_type()
    std::vector<void *> get_row(const size_t &pos);
    /// @brief Adds one row. Converts strings to appropriate types.
    ///        One row means a value from each column.
    /// @returns Returns 0 on success.
    ///          1 - Args vector is too big/small.
    ///          2 - Argument of type 'int' is found to be
    ///              not convertible to int.
    ///          3 - Argument of type 'b_int' is found to be
    ///              not convertible to unsigned long long.
    /// @see get_column_types()
    /// @see get_column_type()
    int add(std::vector<std::string> &args);
    /// @brief Erases element with ID.
    bool erase_id(const size_t &id);
    /// @brief Clears all columns.
    void clear();
    size_t get_column_count() const;
    const std::vector<std::string> &get_column_names() const;
    const std::string &get_column_name(const size_t &pos) const;
    /// @brief Searches for column with specified name in a table.
    /// @returns Position of column in the table. TDB_NOT_FOUND if element
    ///          is not found.
    size_t search_column_index(const std::string &name) const;
    /// @see ToiletType
    const std::vector<int> &get_column_types() const;
    /// @see ToiletType
    const int &get_column_type(const size_t &pos) const;
    size_t get_row_count() const;
    /// @return Suitable ID for a new element.
    size_t get_next_id() const;
};

} // namespace toiletdb

#endif // TOILET_IN_MEMORY_TABLE_H_