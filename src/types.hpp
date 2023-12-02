#ifndef TDB_TYPES_H
#define TDB_TYPES_H

#include <cstdint>
#include <string>
#include <vector>

#include "common.hpp"

#define TDB_PARSER_FORMAT 1
#define TDB_MAGIC "tdb"

namespace toiletdb {

enum class Column_Type
{
    UNKN = 0,
    INT,
    UINT,
    STR,
};

enum class Column_Modifier
{
    CONST = 1 << 0,
    ID = 1 << 1,
};

struct Column_Info
{
    Column_Info() = default;
    Column_Info(std::string_view name, Column_Type type, uint32_t modifiers)
        : type(type), modifiers(modifiers), name(name)
    {}

    void set_type_checked(Column_Type type);
    void add_modifier_checked(Column_Modifier modifier);

    bool is(Column_Modifier modifier) const noexcept;
    bool is(Column_Type type) const noexcept;

    Column_Type type;
    uint32_t modifiers;
    std::string name;
};

class Column_Base
{
public:
    Column_Base(const Column_Info &info)
        : info(info)
    {}

    Column_Type get_type() const noexcept;
    uint32_t get_modifiers() const noexcept;
    std::string_view get_name() const noexcept;

    bool is(Column_Modifier modifier) const noexcept;
    bool is(Column_Type type) const noexcept;

    virtual size_t size() const noexcept = 0;
    virtual void remove(size_t pos) = 0;
    virtual void clear() noexcept = 0;

protected:
    Column_Info info;
};

template <typename T>
class Memory_Column : public Column_Base
{
public:
    Memory_Column(const Column_Info &info)
        : Column_Base(info)
    {}

    Column_Type get_type() const noexcept
    {
        return info.type;
    }

    uint32_t get_modifiers() const noexcept
    {
        return info.modifiers;
    }

    std::string_view get_name() const noexcept
    {
        return info.name;
    }

    void remove(size_t pos)
    {
        return data.erase(pos);
    }

    void clear() noexcept
    {
        return data.clear();
    }

    size_t size() const noexcept
    {
        return data.size();
    }

    void add(const T &data)
    {
        return data.push_back(data);
    }

    T &get_mut(size_t pos)
    {
        return data[pos];
    }

    const T &get(size_t pos) const
    {
        return data[pos];
    }

    std::vector<T> &get_data() const
    {
        return data;
    }

private:
    std::vector<T> data;
};

} // namespace toiletdb

#endif // TDB_TYPES_H
