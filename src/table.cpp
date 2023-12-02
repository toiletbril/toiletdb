#include "table.hpp"
#include "file_manager.hpp"

namespace toiletdb {

Memory_Table::Memory_Table(std::string_view file_path)
    : file_manager(std::make_unique<Memory_File_Manager>(file_path))
{}

void Memory_Table::init()
{
    file_manager->check();
    std::vector<Column_Info> types = file_manager->read_header();

    auto temp_columns = file_manager->deserialize(types);
    columns = std::make_unique<std::vector<Column_Base *>>(temp_columns);
}

void Memory_Table::serialize() const
{
    file_manager->serialize(columns.get());
}

void Memory_Table::serialize(std::string_view file_path) const
{
    file_manager->serialize(columns.get(), file_path);
}


void Memory_Table::deserialize()
{
    std::vector<Column_Info> types = file_manager->read_header();
    file_manager->deserialize(types);
}

void Memory_Table::deserialize(std::string_view file_path)
{
    std::vector<Column_Info> types = file_manager->read_header();
    file_manager->deserialize(types, file_path);
}

std::optional<size_t> Memory_Table::search(size_t id) const
{
    Column_Base *ids =
        static_cast<Memory_Column<size_t> *>((*columns.get())[get_id_column_pos()]);

    // @@@: indexing
    return std::nullopt;
}


std::optional<std::vector<size_t>> Memory_Table::search(size_t column_pos, std::string_view query) const
{
    // @@@: soon tm 
    return std::nullopt;
}

} // namespace toiletdb
