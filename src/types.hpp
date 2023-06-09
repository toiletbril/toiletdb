#ifndef TOILET_TYPES_H_
#define TOILET_TYPES_H_

#include <string>
#include <vector>

#include "debug.hpp"

#include "common.hpp"

#define TOILETDB_PARSER_FORMAT_VERSION 1
#define TOILETDB_MAGIC "tdb"

/// @brief Type mask for ToiletType
#define TDB_TMASK 0b00000111
/// @brief Modifier mask for ToiletType
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
 * Cast a void pointer and dereference it.
 */
#define TDB_CAST(type, prow_value) (*(static_cast<type *>(prow_value)))

namespace toiletdb {

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
 * @brief Structure used in InMemoryParser to store information about columns.
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
 * @brief Internal column type to derive from.
 */
template <typename T>
class Column : public ColumnBase
{
public:
    /// @brief Appends an element to in-memory vector.
    virtual void add(T data) = 0;
    /// @brief Void pointer to a vector member at 'pos'
    virtual T &get(size_t pos) = 0;
    /// @brief Void pointer to the internal vector.
    virtual std::vector<T> &get_data() = 0;
};

class ColumnInt : public Column<int>
{
    std::vector<int> *data;
    std::string name;
    int type;

public:
    ColumnInt(std::string name, int type);
    ~ColumnInt() override;
    const int &get_type() const override;
    const std::string &get_name() const override;
    size_t size() const override;
    void erase(size_t pos) override;
    void clear() override;
    void add(int data) override;
    int &get(size_t pos) override;
    std::vector<int> &get_data() override;
};

class ColumnUint : public Column<size_t>
{
    std::vector<size_t> *data;
    std::string name;
    int type;

public:
    ColumnUint(const std::string name, int type);
    ~ColumnUint() override;
    const int &get_type() const override;
    const std::string &get_name() const override;
    size_t size() const override;
    void erase(size_t pos) override;
    void clear() override;
    void add(size_t data) override;
    size_t &get(size_t pos) override;
    std::vector<size_t> &get_data() override;
};

class ColumnStr : public Column<std::string>
{
    std::vector<std::string> *data;
    std::string name;
    int type;

public:
    ColumnStr(std::string name, int type);
    ~ColumnStr() override;
    const int &get_type() const override;
    const std::string &get_name() const override;
    size_t size() const override;
    void erase(size_t pos) override;
    void clear() override;
    void add(std::string data) override;
    std::string &get(size_t pos) override;
    std::vector<std::string> &get_data() override;
};

} // namespace toiletdb

#endif // TOILET_TYPES_H_
