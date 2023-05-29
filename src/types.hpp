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
 * Cast a void pointer and get it's value.
 */
#define TDB_GET(type, prow_value) (*(static_cast<type *>(prow_value)))
/**
 * Cast Column.get_data() to appropriate vector type
 * and get pointer's value.
 */
#define TDB_GET_DATA(of_type, pcolumn)                                         \
    (*(static_cast<std::vector<of_type> *>((*pcolumn).get_data())))

namespace toiletdb {

/**
 * @brief One bit in 32 bit integer that means either a type or a modifier.
 *        A column can have only one type and any amount of modifiers.
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
    T_CONST = 1 << 4,
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
 */
class Column
{
public:
    virtual ~Column() = 0;
    /// @see ToiletType
    virtual const int &get_type() const         = 0;
    virtual const std::string &get_name() const = 0;
    virtual size_t size() const                 = 0;
    virtual void erase(size_t pos)              = 0;
    virtual void clear()                        = 0;
    /// @brief Appends an element to in-memory vector.
    ///        Type will be casted back in method body.
    ///        I couldn't figure out how to make this more convenient.
    virtual void add(void *data) = 0;
    /// @brief Void pointer to a vector member at 'pos'
    virtual void *get(size_t pos) = 0;
    /// @brief Void pointer to the internal vector.
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
    const int &get_type() const override;
    const std::string &get_name() const override;
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
    const int &get_type() const override;
    const std::string &get_name() const override;
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
    const int &get_type() const override;
    const std::string &get_name() const override;
    size_t size() const override;
    void erase(size_t pos) override;
    void clear() override;
    void add(void *data) override;
    void *get(size_t pos) override;
    void *get_data() override;
};

} // namespace toiletdb

#endif // TOILET_TYPES_H_
