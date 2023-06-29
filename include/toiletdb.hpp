/*
 * Copyright (c) 2023 toiletbril <https://github.com/toiletbril>
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef TOILETDB_H_
#define TOILETDB_H_

#include <algorithm>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#define TOILETDB_VERSION "1.3.1"
#define TOILETDB_PARSER_FORMAT_VERSION 1

#define TDB_INVALID_ULL (size_t)(-1)
#define TDB_NOT_FOUND (size_t)(-1)
#define TDB_INVALID_I 2147483647
/**
 *  @brief Type mask for ToiletType
 */
#define TDB_TMASK 0b00000111
/**
 *  @brief Modifier mask for ToiletType
 */
#define TDB_MMASK 0b00111000

/**
 * @brief Macro to extract a type from type integer specifically for switch
 * statements. switch (TDB_TYPE(type)) { ... };
 */
#define TDB_TYPE(type) (type & TDB_TMASK)
/**
 * @brief Macro that returns boolean if types match.
 *      if (TDB_IS(type, T_INT)) { ... };
 */
#define TDB_IS(type, is_type) (type & is_type)
/**
 * Cast a void pointer and get it's value.
 */
#define TDB_CAST(type, prow_value) (*(static_cast<type *>(prow_value)))

#ifndef NDEBUG
    #define TDB_DEBUGV(v, name) toiletdb::toilet_debug_putv(v, name)
    #define TDB_DEBUGS(s, name) toiletdb::toilet_debug_puts(s, name)
#else
    #define TDB_DEBUGV(v, name)
    #define TDB_DEBUGS(s, name)
#endif

namespace toiletdb {

#ifndef NDEBUG
template <typename T, typename A>
void toilet_debug_putv(const std::vector<T, A> &v, const char *name)
{
    std::cout << "*** " << name << ": [\n";
    for (const T &s : v) {
        std::cout << "\t'" << s << "',\n";
    }
    std::cout << "]\n";
    fflush(stdout);
};

template <typename T>
void toilet_debug_puts(const T &s, const char *name)
{
    std::cout << "*** " << name << ": '" << s << "'\n";
    fflush(stdout);
}
#endif

/**
 *  @brief Parse size_t.
 *  @return TDB_INVALID_ULL if string cannot be parsed.i
 */
size_t parse_long_long(const std::string &str);
/**
 *  @brief Parse signed int.
 *  return TDB_INVALID_I if string cannot be parsed.
 */
int parse_int(const std::string &str);
/**
 *  @brief Returns copy of a string with all characters lowercased.
 */
std::string to_lower_string(const std::string &str);

/**
 * @brief One bit in 32 bit integer that means either a type or a modifier.
 *        A column can have only one type and any amount of modifiers.
 * @see TDB_TYPE
 */
enum ToiletType
{
    /// @brief Signed integer.
    TT_INT = 1 << 0,
    /// @brief Unsigned integer.
    TT_UINT = 1 << 1,
    /// @brief std::string.
    TT_STR = 1 << 2,
    /// @brief Marks column to be used for indexing.
    /// Can only be used on type 'uint'.
    TT_ID = 1 << 3,
    /// @brief Marks column to be constant.
    TT_CONST = 1 << 4,
};

/**
 * @class ParsingError
 * @brief Is thrown when internal parser encounters errors.
 */
class ParsingError : public std::logic_error
{
public:
    ParsingError(std::string const &msg);
};

/**
 * @brief Base class for columns in InMemoryTable table.
 *        This should be casted to appropriate column type.
 */
class ColumnBase
{
public:
    virtual ~ColumnBase(){};
    /// @see ToiletType
    virtual const int &get_type() const         = 0;
    virtual const std::string &get_name() const = 0;
    virtual size_t size() const                 = 0;
    virtual void clear()                        = 0;
    virtual void erase(size_t pos)              = 0;
};

/**
 * @class InMemoryTable
 * @brief Represents one table.
 *        Each table represents one file.
 */
class InMemoryTable
{
private:
    struct Private;

public:
    /// @brief Opens up a file and loads it up into memory.
    /// @warning Does not create a file. Will throw an error.
    /// @throws std::ios::failure when file cannot be opened.
    /// @throws std::runtime_error when file does not exist.
    /// @throws ParsingError when parsing error is encountered.
    InMemoryTable(const std::string &filename);
    ~InMemoryTable();
    const std::vector<ColumnBase *> &get_all() const;
    /// @brief Discards all changes made to in-memory vector, and reads file
    ///        again.
    /// @throws std::runtime_error when table file was deleted or moved.
    void reread_file();
    /// @brief Writes data stored in memory back to the file.
    /// @throws std::runtime_error when table file was deleted or moved.
    void write_file() const;
    /// @brief Search in-memory vector by ID.
    ///        O(log n)
    /// @return TDB_NOT_FOUND if element is not found.
    size_t search(const size_t &id) const;
    /// @brief Search in-memory vector by comparing values as std::string.
    ///        O(n)
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
    /// is not found.
    size_t search_column_index(const std::string &name) const;
    /// @see ToiletType
    const std::vector<int> &get_types() const;
    /// @see ToiletType
    const int &get_column_type(const size_t &pos) const;
    size_t get_row_count() const;
    /// @return Suitable ID for a new element.
    size_t get_next_id() const;
};

}; // namespace toiletdb

#endif // TOILETDB_H_
