#include "types.hpp"

namespace toiletdb {

void Column_Info::set_type_checked(Column_Type type)
{
    if (this->type != Column_Type::UNKN) {
        throw "Column_Info::set_type_checked: type was already set";
    }

    this->type = type;
}

void Column_Info::add_modifier_checked(Column_Modifier modifier)
{
    if (this->modifiers & static_cast<uint32_t>(modifier)) {
        throw "Column_Info::set_modifier_checked: modifier was already set";
    }

    this->modifiers |= static_cast<uint32_t>(modifier);
}

Column_Type Column_Base::get_type() const noexcept
{
    return this->info.type;
}

uint32_t Column_Base::get_modifiers() const noexcept
{
    return this->info.modifiers;
}

std::string_view Column_Base::get_name() const noexcept
{
    return this->info.name;
}

bool Column_Info::is(Column_Modifier modifier) const noexcept
{
    return (this->modifiers & static_cast<uint32_t>(modifier));
}

bool Column_Info::is(Column_Type type) const noexcept
{
    return (this->type == type);
}

} // namespace toiletdb
