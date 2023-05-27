#ifndef TOILETDB_H_
#define TOILETDB_H_

#include <algorithm>
#include <cctype>
#include <climits>
#include <cmath>
#include <cstring>
#include <fstream>
#include <iostream>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

#define TOILETDB_VERSION "1.0.1"

#define TDB_NOT_FOUND (size_t)(-1)

#define TDB_INVALID_ULL (size_t)(-1)
#define TDB_INVALID_I 2147483647

#define TOILET_TYPES_H_

#define TOILETDB_PARSER_VERSION 1
#define TOILETDB_MAGIC "tdb"

/// @brief Type mask for ToiletType
#define TDB_TMASK 0b00000111
/// @brief Modifier mask for ToiletType
#define TDB_MMASK 0b00111000

/**
 * @brief Macro to extract a type from type integer.
 *
 * Example:
 *      int type = ...;
 *      if (TDB_TYPE(type) == T_INT) { ... };
 */
#define TDB_TYPE(type) (type & TDB_TMASK)

namespace toiletdb {

/// @brief Parse unsigned long long.
/// @return TDB_INVALID_ULL if string cannot be parsed.
size_t cm_parsell(std::string &str);

/// @brief Parse signed int.
/// @return TDB_INVALID_I if string cannot be parsed.
int cm_parsei(std::string &str);

/// @brief Returns copy of a string with all characters lowercased.
std::string cm_str_tolower(std::string &str);

/// @brief Replaces all characters in a string with their lowercase variants.
void cm_pstr_tolower(std::string &str);

template <typename T, typename A>
void debug_putv(const std::vector<T, A> &v, const char *name);

template <typename T> void debug_puts(const T &s, const char *name);

struct FormatOne
{
    static size_t read_version(std::fstream &file);

    static TableInfo read_types(std::fstream &file);

    static std::vector<Column *> deserealize(std::fstream &file,
                                             TableInfo &columns,
                                             std::vector<std::string> &names);

    static void write_header(std::fstream &file,
                             const std::vector<Column *> &data);

    static void serialize(std::fstream &file,
                          const std::vector<Column *> &data);
};

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
    /// @returns Position of column in the table. TDB_NOT_FOUND if element
    /// is not found.
    size_t search_column_index(const std::string &name) const;

    /// @see ToiletType
    const std::vector<int> get_column_types() const;

    size_t total_rows() const;

    size_t get_next_id() const;
};

/**
 * @brief One bit in 32 bit integer that meeans either a type or a modifier.
 * A column can have only one type and any amount of modifiers.
 *
 * @see TDB_TYPE
 */
enum ToiletType
{
    /// @brief 32 bit signed integer.
    T_INT = 1 << 0,
    /// @brief 64 bit unsigned integer.
    T_B_INT = 1 << 1,
    /// @brief std::string.
    T_STR = 1 << 2,
    /// @brief Marks column to be used for indexing.
    /// Can only be used on type 'b_int'.
    T_ID = 1 << 3,
    /// @brief Marks column to be constant.
    T_CONST = 1 << 4, // const
};

/**
 * @brief Structure used in InMemoryParser to store information about
 * columns.
 */
struct TableInfo
{
    size_t size;
    size_t id_field_index;
    std::vector<std::string> names;
    std::vector<int> types;
};

/**
 * @brief Base class for columns in InMemoryTable table.
 */
class Column
{
public:
    /// @see ToiletType
    virtual int get_type() const         = 0;
    virtual std::string get_name() const = 0;
    virtual size_t size() const          = 0;
    virtual void erase(size_t pos)       = 0;
    virtual void clear()                 = 0;
    /// @brief Appends an element to in-memory vector.
    /// Type will be casted back in method body.
    /// NOTE: I couldn't figure out how to make this more convenient.
    virtual void add(void *data) = 0;
    /// @brief Void pointer to a vector member at 'pos'
    virtual void *get(size_t pos) = 0;
    /// @brief Void pointer to a in-memory vector.
    virtual void *get_data() = 0;
};

class ColumnInt : public Column
{
    std::vector<int> *data;
    std::string name;
    int type;

public:
    ColumnInt(std::string name, int type);
    ~ColumnInt();
    int get_type() const override;
    std::string get_name() const override;
    size_t size() const override;
    void erase(size_t pos) override;
    void clear() override;
    void add(void *data) override;
    void *get(size_t pos) override;
    void *get_data() override;
};

class ColumnB_Int : public Column
{
    std::vector<unsigned long long> *data;
    std::string name;
    int type;

public:
    ColumnB_Int(const std::string name, int type);
    ~ColumnB_Int();
    int get_type() const override;
    std::string get_name() const override;
    size_t size() const override;
    void erase(size_t pos) override;
    void clear() override;
    void add(void *data) override;
    void *get(size_t pos) override;
    void *get_data() override;
};

class ColumnStr : public Column
{
    std::vector<std::string> *data;
    std::string name;
    int type;

public:
    ColumnStr(std::string name, int type);
    ~ColumnStr();
    int get_type() const override;
    std::string get_name() const override;
    size_t size() const override;
    void erase(size_t pos) override;
    void clear() override;
    void add(void *data) override;
    void *get(size_t pos) override;
    void *get_data() override;
};

}; // namespace toiletdb

#endif // TOILET_TYPES_H_