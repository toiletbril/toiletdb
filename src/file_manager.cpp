#include "file_manager.hpp"
#include "common.hpp"
#include "table_manip.hpp"

#include <string>
#include <span>
#include <fstream>

#include <cstdlib>
#include <cstdio>

namespace toiletdb {

Memory_File_Manager::Memory_File_Manager(std::string_view file_path)
    : file_path(file_path)
{}

std::vector<Column_Info> Memory_File_Manager::read_header()
{
    if (!file_exists(file_path)) {
        throw "Memory_File_Manager::read_version:" + file_path + "does not exist";
    }

    std::fstream file = open(std::ios::in);

    std::string line_buffer;
    header_size = 0;

    std::getline(file, line_buffer);
    header_size += line_buffer.size();

    format_version = read_version_row(line_buffer);

    std::getline(file, line_buffer);
    header_size += line_buffer.size();

    std::vector<Column_Info> types = read_types_row(line_buffer);

    return types;
}

std::fstream Memory_File_Manager::open(std::ios_base::openmode open_mode)
{
    return std::fstream(file_path, open_mode);
}

void Memory_File_Manager::serialize(std::span<const Column_Base *> columns)
{
    std::fstream file = open(std::ios::out | std::ios::trunc);

    if (!file.is_open()) {
        throw "Memory_File_Manager::serialize: Could not open " + file_path;
    }

    file << make_version_row(format_version) << "\n";
    file << make_types_row(columns) << "\n";

    if (columns.empty()) {
        throw "Memory_File_Manager::serialize: 'columns' is empty";
    }
    size_t row_count = columns[0]->size();

     for (size_t row = 0; row < row_count; ++row) {
        for (size_t col = 0; col < columns.size(); ++col) {
            file << '|';

            const Column_Base *column = columns[col];

            switch (column->get_type()) {
                case Column_Type::INT:
                    file << (static_cast<const Memory_Column<int> *>(column))->get(row);
                    break;

                case Column_Type::UINT:
                    file << (static_cast<const Memory_Column<size_t> *>(column))->get(row);
                    break;

                case Column_Type::STR:
                    file << (static_cast<const Memory_Column<std::string> *>(column))->get(row);
                    break;

                case Column_Type::UNKN:
                    throw "Memory_File_Manager::serialize: unreachable";
            }
        }
        file << "|\n";
    }

    file.close();
}

std::vector<Column_Base *> Memory_File_Manager::deserialize(std::span<const Column_Info> types)
{
    if (header_size == 0) {
        throw "Memory_File_Manager::deserialize: file has not been read yet";
    }

    std::vector<Column_Base *> columns;

    for (size_t i = 0; i < types.size(); ++i) {
        Column_Base *c;
        const Column_Info type = types[i];

        switch (type.type) {
            case Column_Type::INT: {
                c = new Memory_Column<int>(type);
                columns.push_back(c);
            } break;
            case Column_Type::UINT: {
                c = new Memory_Column<size_t>(type);
                columns.push_back(c);
            } break;
            case Column_Type::STR: {
                c = new Memory_Column<std::string>(type);
            } break;
            case Column_Type::UNKN:
                throw "Memory_File_Manager::deserialize: unreachable";
        }

        columns.push_back(c);
    }

    std::ifstream file = std::ifstream(file_path, std::ios::in);
    if (!file.is_open()) {
        throw "deserialize: Could not open " + file_path;
    }

    std::string string_buffer;

    // Data starts from the third line
    size_t current_line = 3;

    while (file.eofbit) {
        std::getline(file, string_buffer);
        std::vector<std::string> row = split_row(string_buffer, types.size());

        size_t column_index = 0;
        for (std::string_view value : row) {
            switch (columns[column_index]->get_type()) {
                case Column_Type::INT: {
                    auto column = static_cast<Memory_Column<int> *>(columns[column_index]);

                    std::optional<int> parsed_value = parse_signed_int(value);
                    if (!parsed_value) {
                        throw "Memory_File_Manager::deserialize: unable to parse " + std::string(value) + " as an integer "
                              "at line " + std::to_string(current_line);
                    }

                    column->add(parsed_value.value());
                } break;
                case Column_Type::UINT: {
                    auto column = static_cast<Memory_Column<size_t> *>(columns[column_index]);

                    std::optional<size_t> parsed_value = parse_size_t(value);
                    if (!parsed_value) {
                        throw "Memory_File_Manager::deserialize: unable to parse " + std::string(value) + " as an unsigned integer"
                              "at line " + std::to_string(current_line);
                    }

                    column->add(parsed_value.value());
                } break;
                case Column_Type::STR: {
                    auto column = static_cast<Memory_Column<std::string> *>(columns[column_index]);
                    std::string parsed_value(value);
                    column->add(parsed_value);
                } break;
                case Column_Type::UNKN:
                    throw "Memory_File_Manager::deserialize: unreachable";
            }
            ++current_line;
            ++column_index;
        }
    }

    return columns;
}

}
