#include <fstream>
#include <vector>

#include "format.hpp"
#include "debug.hpp"

namespace toiletdb {

// Reads first line of a file and updates InMemoryFileParser format version
size_t FormatOne::read_version(std::fstream &file)
{
    // Check first line of the file.
    std::string temp;
    std::getline(file, temp);

    // Check for magic string
    for (int i = 0; i < 3; ++i) {
        if (temp[i] != TOILETDB_MAGIC[i]) {
            std::string failstring = "Database file format is not "
                                     "correct: File is not a tdb database";

            throw ParsingError(failstring);
        }
    }

    // Check version
    int len = temp.size();
    std::string version_string;

    for (int i = 3; i < len; ++i) {
        if (temp[i] == '\r') {
            continue;
        }
        version_string += temp[i];
    }

    size_t version = parse_long_long(version_string);

    if (version == TDB_INVALID_ULL) {
        throw ParsingError("Database format is not "
                           "correct: Invalid version number");
    }

    if (version > TOILETDB_PARSER_FORMAT_VERSION) {
        throw ParsingError("Database format is not "
                           "supported: Version is too new");
    };

    TOILET_DEBUGS(version, "InMemoryFileParser.read_version");

    return version;
}

TableInfo FormatOne::read_types(std::fstream &file)
{
    TableInfo fields;
    std::vector<std::string> names;

    int field_n = 0;
    size_t pos  = 1;

    size_t id_pos = 0;

    char c = file.get();

    if (c != '|') {
        throw ParsingError("Format error: Line 2 does not begin with |");
    }

    c = file.get();
    ++pos;

    bool set_id = false;

    while (c != '\n') {
        if (c == '\r') {
            c = file.get();
            continue;
        }
        int type = 0;

        std::string buf;

        bool set_type = false;

        while (c != '|') {
            if (c == ' ') {
                if (buf == "int" && !set_type) {
                    type |= (int)T_INT;
                    set_type = true;
                }
                else if (buf == "b_int" && !set_type) {
                    type |= (int)T_B_INT;
                    set_type = true;
                }
                else if (buf == "str" && !set_type) {
                    type |= (int)T_STR;
                    set_type = true;
                }
                else if (buf == "const") {
                    type |= (int)T_CONST;
                }
                else if (buf == "id") {
                    if (set_id) {
                        std::string failstring =
                            "Format error: ID is already set at 2:" +
                            std::to_string(pos) +
                            ", previous ID at 2:" + std::to_string(id_pos);

                        throw ParsingError(failstring);
                    }

                    type |= (int)T_ID;

                    fields.id_field_index = field_n;

                    id_pos = pos;
                    set_id = true;
                }
                else if (!set_type) {
                    std::string failstring =
                        "Format error: Unknown type modifier at 2:" +
                        std::to_string(pos);
                    throw ParsingError(failstring);
                }

                buf.clear();
            }
            else {
                buf += c;
            }

            c = file.get();
            ++pos;
        }

        // Last word will be the name
        if (buf.empty()) {
            std::string failstring = "Format error: No column name at "
                                     "2:" +
                                     std::to_string(pos);
            throw ParsingError(failstring);
        }

        names.push_back(buf);

        fields.types.push_back(type);
        ++field_n;

        c = file.get();
        ++pos;
    }

#ifndef NDEBUG
    TOILET_DEBUGV(fields.types, "InMemoryFileParser.update_types types");
#endif

    fields.size  = field_n;
    fields.names = names;

    return fields;
}

std::vector<Column *> FormatOne::deserealize(std::fstream &file,
                                             TableInfo &columns,
                                             std::vector<std::string> &names)
{
    // Allocate memory for each field.

    std::vector<Column *> parsed_columns;
    parsed_columns.reserve(columns.size);

    if (names.size() != columns.size) {
        std::string failstring = "In FormatOne.deserialize(), names.size() (" +
                                 std::to_string(names.size()) +
                                 ") is not equal to columns.size (" +
                                 std::to_string(columns.size) + ")";

        throw ParsingError(failstring);
    }

    for (size_t i = 0; i < columns.size; ++i) {
        int type = columns.types[i];

        if (type & T_INT) {
            Column *v = new ColumnInt(names[i], type);
            parsed_columns.push_back(v);
        }
        else if (type & T_B_INT) {
            Column *v = new ColumnB_Int(names[i], type);
            parsed_columns.push_back(v);
        }
        else if (type & T_STR) {
            Column *v = new ColumnStr(names[i], type);
            parsed_columns.push_back(v);
        }
    }

    std::string temp;

    // Data starts from third line.
    // First line is magic, second is types.
    size_t line = 3;
    size_t pos  = 1;

    bool debug_crlf = false;
    int c = file.get();

    while (c != EOF) {
        if (c != '|') {
            std::string failstring = "Database file format is not "
                                     "correct: Invalid delimiter at line " +
                                     std::to_string(line) + ":" +
                                     std::to_string(pos);

            throw ParsingError(failstring);
        }
        else {
            c = file.get();
        }

        std::vector<std::string> fields;
        fields.reserve(columns.size);
        size_t field = 0;

        while (c != '\n') {
            if (c == '\r') {
                if (!debug_crlf) {
                    TOILET_DEBUGS("CRLF detected",
                               "InMemoryFileParser.deserialize");
                    debug_crlf = true;
                }

                c = file.get();
                continue;
            }

            if (field > columns.size) {
                std::string failstring = "Database file format is not "
                                         "correct: Extra field at line " +
                                         std::to_string(line) + ":" +
                                         std::to_string(pos + 1);

                throw ParsingError(failstring);
            }

            if (c == '|') {
                fields.push_back(temp);
                temp.clear();
                ++field;
            }
            else {
                temp += c;
            }

            c = file.get();
            ++pos;
        }

        if (field != columns.size) {
            std::string failstring =
                "Database file format is not "
                "correct: Invalid number of fields (" +
                std::to_string(columns.size) + " required, actual " +
                std::to_string(field) + ") at line " + std::to_string(line) +
                ":" + std::to_string(pos);

            throw ParsingError(failstring);
        }

#ifndef NDEBUG
        TOILET_DEBUGV(fields, "InMemoryFileParser.deserealize");
#endif

        for (size_t i = 0; i < columns.size; ++i) {
            if (columns.types[i] & T_ID) {
                if (!(columns.types[i] & T_B_INT)) {
                    std::string failstring = "Database file format is not "
                                             "correct: Field with modifier "
                                             "'id' is not of type 'b_int', "
                                             "line " +
                                             std::to_string(line) + ", field " +
                                             std::to_string(i + 1);

                    throw ParsingError(failstring);
                }
            }
            if (columns.types[i] & T_INT) {
                size_t num = parse_int(fields[i]);

                if (num == TDB_INVALID_I) {
                    std::string failstring =
                        "Database file format is not "
                        "correct: Field of type 'int' is not a number, "
                        "line " +
                        std::to_string(line) + ", field " +
                        std::to_string(i + 1);

                    throw ParsingError(failstring);
                }

                static_cast<std::vector<int> *>(parsed_columns[i]->get_data())
                    ->push_back(num);
            }
            else if (columns.types[i] & T_B_INT) {
                size_t num = parse_long_long(fields[i]);

                if (num == TDB_INVALID_ULL) {
                    std::string failstring =
                        "Database file format is not "
                        "correct: Field of type 'b_int' is not a number, "
                        "line " +
                        std::to_string(line) + ", field " +
                        std::to_string(i + 1);

                    throw ParsingError(failstring);
                }

                static_cast<std::vector<unsigned long long> *>(
                    parsed_columns[i]->get_data())
                    ->push_back(num);
            }

            else if (columns.types[i] & T_STR) {
                static_cast<std::vector<std::string> *>(
                    parsed_columns[i]->get_data())
                    ->push_back(fields[i]);
            }
        }

        c = file.get();
        ++line;
        pos = 1;
    }

    return parsed_columns;
}

void FormatOne::write_header(std::fstream &file,
                             const std::vector<Column *> &data)
{
    std::string header = "tdb1\n";

    for (const Column *c : data) {
        header += '|';

        int modifiers = c->get_type();

        // Write modifiers
        if (modifiers & T_ID) {
            header += "id ";
        }

        if (modifiers & T_CONST) {
            header += "const ";
        }

        // Write type
        switch (modifiers & TDB_TMASK) {
            case T_INT: {
                header += "int ";
            } break;

            case T_B_INT: {
                header += "b_int ";
            } break;

            case T_STR: {
                header += "str ";
            } break;
        }

        // Write name. No space at the end.
        header += c->get_name();
    }
    header += '|';

    TOILET_DEBUGS(header, "InMemoryFileParser.write_header");

    file << header << std::endl;
}

void FormatOne::serialize(std::fstream &file, const std::vector<Column *> &data)
{
    FormatOne::write_header(file, data);

    size_t column_count = data.size();

    if (!data[0]) {
        ParsingError("In FormatOne.serialize(), there is no elements in data");
    }

    size_t row_count = data[0]->size();

    std::vector<int> types;
    types.reserve(column_count);

    for (const Column *c : data) {
        types.push_back(c->get_type());
    }

    for (size_t row = 0; row < row_count; ++row) {
        for (size_t col = 0; col < column_count; ++col) {
            file << '|';
            Column *c  = data[col];
            void *item = c->get(row);

            switch (c->get_type() & TDB_TMASK) {
                case T_INT: {
                    file << *(static_cast<int *>(item));
                } break;

                case T_B_INT: {
                    file << *(static_cast<unsigned long long *>(item));
                } break;

                case T_STR: {
                    file << *(static_cast<std::string *>(item));
                } break;
            }
        }
        file << "|\n";
    }
    TOILET_DEBUGS(row_count, "InMemoryFileParser.serialize rows saved");
}

} // namespace toiletdb
