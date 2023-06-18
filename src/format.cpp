#include <fstream>
#include <vector>

#include "debug.hpp"
#include "format.hpp"

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
            std::string failstring =
                "Database file format is not "
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
        throw ParsingError(
            "Database format is not "
            "correct: Invalid version number");
    }

    if (version > TOILETDB_PARSER_FORMAT_VERSION) {
        throw ParsingError(
            "Database format is not "
            "supported: Version is too new");
    };

    TDB_DEBUGS(version, "InMemoryFileParser.read_version");

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
                    type |= (int)TT_INT;
                    set_type = true;
                }
                else if (buf == "uint" && !set_type) {
                    type |= (int)TT_UINT;
                    set_type = true;
                }
                else if (buf == "str" && !set_type) {
                    type |= (int)TT_STR;
                    set_type = true;
                }
                else if (buf == "const") {
                    type |= (int)TT_CONST;
                }
                else if (buf == "id") {
                    if (set_id) {
                        std::string failstring =
                            "Format error: ID is already set at 2:" +
                            std::to_string(pos) +
                            ", previous ID at 2:" + std::to_string(id_pos);

                        throw ParsingError(failstring);
                    }

                    type |= (int)TT_ID;

                    fields.id_field_index = field_n;

                    id_pos = pos;
                    set_id = true;
                }
                else if (!set_type) {
                    std::string failstring =
                        "Format error: Unknown type modifier '" + buf + "' at 2:" +
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
            std::string failstring =
                "Format error: No column name at "
                "2:" +
                std::to_string(pos);

            throw ParsingError(failstring);
        }

        // Allow T_ID only for T_B_INT
        if (TDB_IS(type, TT_ID) && !TDB_IS(type, TT_UINT)) {
            std::string failstring =
                "Format error: ID column is not of type 'uint' at "
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

    TDB_DEBUGV(fields.types, "InMemoryFileParser.update_types types");

    fields.size  = field_n;
    fields.names = names;

    return fields;
}

std::vector<ColumnBase *> FormatOne::deserealize(std::fstream &file,
                                                 TableInfo &columns,
                                                 std::vector<std::string> &names)
{
    // Allocate memory for each field.

    std::vector<ColumnBase *> parsed_columns;
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

        if (type & TT_INT) {
            ColumnBase *v = new ColumnInt(names[i], type);
            parsed_columns.push_back(v);
        }
        else if (type & TT_UINT) {
            ColumnBase *v = new ColumnUint(names[i], type);
            parsed_columns.push_back(v);
        }
        else if (type & TT_STR) {
            ColumnBase *v = new ColumnStr(names[i], type);
            parsed_columns.push_back(v);
        }
    }

    std::string temp;

    // Data starts from third line.
    // First line is magic, second is types.
    size_t line = 3;
    size_t pos  = 1;

    bool debug_crlf = false;
    int c           = file.get();

    while (c != EOF) {
        if (c != '|') {
            std::string failstring =
                "Database file format is not "
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
                    TDB_DEBUGS("CRLF detected",
                               "InMemoryFileParser.deserialize");
                    debug_crlf = true;
                }

                c = file.get();
                continue;
            }

            if (field > columns.size) {
                std::string failstring =
                    "Database file format is not "
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

        TDB_DEBUGV(fields, "InMemoryFileParser.deserealize");

        for (size_t i = 0; i < columns.size; ++i) {
            if (columns.types[i] & TT_ID) {
                if (!(columns.types[i] & TT_UINT)) {
                    std::string failstring =
                        "Database file format is not "
                        "correct: Field with modifier "
                        "'id' is not of type 'uint', "
                        "line " +
                        std::to_string(line) + ", field " +
                        std::to_string(i + 1);

                    throw ParsingError(failstring);
                }

                // TODO: Reindex when editing ID
                if (!(columns.types[i] & TT_CONST)) {
                    std::string failstring =
                        "Database file format is not "
                        "correct: Field with modifier "
                        "'id' is not constant, "
                        "line " +
                        std::to_string(line) + ", field " +
                        std::to_string(i + 1);

                    throw ParsingError(failstring);
                }
            }
            if (columns.types[i] & TT_INT) {
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

                static_cast<ColumnInt *>(parsed_columns[i])->get_data().push_back(num);
            }
            else if (columns.types[i] & TT_UINT) {
                size_t num = parse_long_long(fields[i]);

                if (num == TDB_INVALID_ULL) {
                    std::string failstring =
                        "Database file format is not "
                        "correct: Field of type 'uint' is not a number, "
                        "line " +
                        std::to_string(line) + ", field " +
                        std::to_string(i + 1);

                    throw ParsingError(failstring);
                }

                (static_cast<ColumnUint *>(parsed_columns[i]))->get_data().push_back(num);
            }

            else if (columns.types[i] & TT_STR) {
                (static_cast<ColumnStr *>(parsed_columns[i]))->get_data().push_back(fields[i]);
            }
        }

        c = file.get();
        ++line;
        pos = 1;
    }

    return parsed_columns;
}

void FormatOne::write_header(std::fstream &file,
                             const std::vector<ColumnBase *> &data)
{
    std::string header = "tdb1\n";

    for (const ColumnBase *c : data) {
        header += '|';

        int modifiers = c->get_type();

        // Write modifiers
        if (modifiers & TT_ID) {
            header += "id ";
        }

        if (modifiers & TT_CONST) {
            header += "const ";
        }

        // Write type
        switch (modifiers & TDB_TMASK) {
            case TT_INT: {
                header += "int ";
            } break;

            case TT_UINT: {
                header += "uint ";
            } break;

            case TT_STR: {
                header += "str ";
            } break;
        }

        // Write name. No space at the end.
        header += c->get_name();
    }
    header += '|';

    TDB_DEBUGS(header, "InMemoryFileParser.write_header");

    file << header << std::endl;
}

void FormatOne::serialize(std::fstream &file, const std::vector<ColumnBase *> &data)
{
    FormatOne::write_header(file, data);

    size_t column_count = data.size();

    if (!data[0]) {
        ParsingError("In FormatOne.serialize(), there is no elements in data");
    }

    size_t row_count = data[0]->size();

    std::vector<int> types;
    types.reserve(column_count);

    for (const ColumnBase *c : data) {
        types.push_back(c->get_type());
    }

    for (size_t row = 0; row < row_count; ++row) {
        for (size_t col = 0; col < column_count; ++col) {
            file << '|';
            ColumnBase *c = data[col];

            switch (c->get_type() & TDB_TMASK) {
                case TT_INT: {
                    file << (static_cast<ColumnInt *>(c))->get(row);
                } break;

                case TT_UINT: {
                    file << (static_cast<ColumnUint *>(c))->get(row);
                } break;

                case TT_STR: {
                    file << (static_cast<ColumnStr *>(c))->get(row);
                } break;
            }
        }
        file << "|\n";
    }

    TDB_DEBUGS(row_count, "InMemoryFileParser.serialize rows saved");
}

} // namespace toiletdb
