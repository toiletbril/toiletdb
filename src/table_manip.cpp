#include "table_manip.hpp"

#include <algorithm>
#include <string>
#include <ranges>

#include "common.hpp"
#include "types.hpp"

namespace toiletdb {

size_t read_version_row(std::string_view line)
{
    if (line.size()) {
        throw "read_version: line is less than 3 characters";
    }

    std::string_view magic = line.substr(0, 3);

    if (magic != TDB_MAGIC) {
        throw "read_version: invalid magic number";
    }

    std::string_view version = line.substr(3);
    std::optional<size_t> version_result = parse_size_t(version);

    if (!version_result) {
        throw "read_version: unable to parse version number";
    }

    return version_result.value();
}

#define STR_SWITCH(str, first_case) if (str == first_case)
#define STR_CASE(str, sub_case) else if (str == sub_case)

std::vector<Column_Info> read_types_row(std::string_view line)
{
    std::vector<Column_Info> info;

    std::string token_buffer;
    size_t id_position = 0;

    size_t pos = 0;

    char c = line.at(pos++);

    if (c != '|') {
        throw "read_types: types do not begin with '|'";
    }

    c = line.at(pos++);

    bool set_id = false;

    while (c != '\n' && c != EOF) {
        if (c == '\r') {
            c = line.at(pos++);
            continue;
        }

        Column_Info column_info;

        while (c != '|') {
            if (c == ' ') {
                STR_SWITCH(token_buffer, "int") {
                    column_info.set_type_checked(Column_Type::INT);
                } STR_CASE(token_buffer, "uint") {
                    column_info.set_type_checked(Column_Type::UINT);
                } STR_CASE(token_buffer, "str") {
                    column_info.set_type_checked(Column_Type::STR);
                } STR_CASE(token_buffer, "const") {
                    column_info.add_modifier_checked(Column_Modifier::CONST);
                } STR_CASE(token_buffer, "id") {
                    if (set_id) {
                        throw "Format error: ID is already set at 2:" + std::to_string(pos) + ", "
                              "previous ID at 2:" + std::to_string(id_position);
                    }

                    column_info.add_modifier_checked(Column_Modifier::ID);

                    id_position = pos;
                    set_id = true;
                } else if (column_info.type == Column_Type::UNKN) {
                    throw "Format error: Unknown type modifier '" + token_buffer + "' "
                          "at 2:" + std::to_string(pos);
                }

                token_buffer.clear();
            }
            else {
                token_buffer += c;
            }

            c = line.at(pos++);
        }

        std::string_view column_name = token_buffer;

        if (column_name.empty()) {
            throw "Format error: No column name at 2:" + std::to_string(pos);
        }

        // ID can only be 'const uint'
        if (column_info.is(Column_Modifier::ID)) {
            if (!column_info.is(Column_Type::UINT)) {
                throw  "read_types: ID column is not 'uint' at 2:" + std::to_string(pos);
            }
            if (!column_info.is(Column_Modifier::CONST)) {
                throw  "read_types: ID column is not 'const' at 2:" + std::to_string(pos);
            }
        }

        if (!set_id) {
            throw "read_types: table requires a column with 'id' modifier";
        }

        size_t duplicate_index =
            std::find_if(info.begin(), info.end(), [&column_name](const Column_Info &i) {
                    return i.name == column_name;
                }) != info.end();

        if (duplicate_index) {
            throw "read_types: duplicate column '" + token_buffer + "' at 2:" + std::to_string(pos) + ", "
                  "previous column is on position " + std::to_string(duplicate_index) + ".";
        }

        info.push_back(column_info);

        c = line.at(pos++);
    }

    TDB_TRACE_VECTOR(info, "read_types types");

    return info;
}

std::vector<std::string> split_row(std::string_view line, size_t column_count)
{
    std::vector<void *> row(column_count);

    size_t pos = 0;

    char c = line.at(pos++);
    if (c != '|') {
        throw "split_row: types do not begin with '|' "
              "at char " + std::to_string(pos);
    }

    c = line.at(pos++);

    std::vector<std::string> fields(column_count);

    std::string token_buffer;
    size_t field_number = 0;

    while (c != '\n' && c != EOF) {
        if (column_count <= field_number) {
            throw "split_row: extra column at char " + std::to_string(pos);
        }

        if (c == '\r') {
            c = line.at(pos++);
            continue;
        }

        if (c == '|') {
            fields.push_back(token_buffer);
            token_buffer.clear();
            ++field_number;
        }
        else {
            token_buffer += c;
        }

        c = line.at(pos++);
    }

    if (field_number < column_count) {
        throw "split_row: less fields than required "
              "(" + std::to_string(column_count) + " required, actual " + std::to_string(field_number) + ") "
              "at char " + std::to_string(pos);
    }

    return fields;
}

std::string make_version_row(size_t version)
{
    return TDB_MAGIC + std::to_string(TDB_PARSER_FORMAT) + "\n";
}

std::string make_types_row(std::span<const Column_Base *> columns)
{
    std::string types = "|";

    for (const Column_Base *column : columns) {
        if (column->is(Column_Modifier::ID)) {
            types += "id ";
        }
        if (column->is(Column_Modifier::CONST)) {
            types += "const ";
        }

        switch (column->get_type()) {
            case Column_Type::INT:
                types += "int ";
                break;

            case Column_Type::UINT:
                types += "uint ";
                break;

            case Column_Type::STR:
                types += "str ";
                break;

            case Column_Type::UNKN:
                throw "make_types: unreachable";
        }

        types += column->get_name();
        types += "|\n";
    }

    return types;
}

} // namespace toiletdb
