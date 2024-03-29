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
#include "errors.hpp"
#include "parser.hpp"
#include "types.hpp"

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
    std::unique_ptr<Private> internal;

public:
    /// @brief Opens up a file and loads it up into memory.
    /// @warning Does not create a file. Will throw an error.
    /// @throws std::ios::failure when file cannot be opened.
    /// @throws std::runtime_error when file does not exist.
    /// @throws ParsingError when parsing error is encountered.
    InMemoryTable(const std::string &filename);
    ~InMemoryTable();
    /// @brief Discards all changes made to in-memory vector, and reads file
    /// again.
    /// @throws std::runtime_error when table file was deleted or moved.
    void reread_file();
    /// @brief Writes data stored in memory back to the file.
    /// @throws std::runtime_error when table file was deleted or moved.
    void write_file() const;
    /// @brief Writes data stored in memory back to the file specified.
    void write_file(const std::string &filepath) const;
    /// @brief Search in-memory vector by ID.
    /// O(log n)
    /// @return TDB_NOT_FOUND if element is not found.
    size_t search(const size_t &id) const;
    /// @brief Search in-memory vector by comparing values as std::string.
    /// O(n)
    /// @return TDB_NOT_FOUND if element is not found.
    std::vector<size_t> search(const std::string &name,
                               std::string &query) const;
    /// @brief Get copy of a row from vector as strings.
    ///        One row means a value from each column.
    const std::vector<std::string> get_row(const size_t &pos) const;
    /// @brief Get one row from vector.
    ///        One row means a value from each column.
    /// @warning You will need to get types and cast them yourself.
    /// @see get_types()
    /// @see get_column_type()
    std::vector<void *> unsafe_get_mut_row(const size_t &pos);
    /// @brief Adds one row. Converts strings to appropriate types.
    ///        One row means a value from each column *EXCEPT* ID.
    /// @warning ID row for new entry will be set automatically.
    ///          When passing values, just skip the ID column.
    /// @returns Returns 0 on success.
    ///          1 - Args vector is too big/small.
    ///          2 - Argument of type 'int' is found to be
    ///              not convertible to int.
    ///          3 - Argument of type 'uint' is found to be
    ///              not convertible to size_t.
    /// @see get_types()
    /// @see get_column_type()
    int add_row(std::vector<std::string> &args);
    /// @brief Erases element with ID.
    bool erase_id(const size_t &id);
    /// @brief Erases element at pos.
    bool erase(const size_t &pos);
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
    const std::vector<int> &get_types() const;
    /// @see ToiletType
    const int &get_column_type(const size_t &pos) const;
    size_t get_row_count() const;
    /// @return Suitable ID for a new element.
    size_t get_next_id() const;
};

} // namespace toiletdb

#endif // TOILET_IN_MEMORY_TABLE_H_
