#pragma once

#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#ifdef DEBUG
    #include "../debug.cpp"
#endif

#include "../common.cpp"

#include "format.cpp"
#include "types.cpp"

// File format:
// 1    tdb002
// 2    |[modifier] <type> <name>|...
//
// Modifiers: ID (id), constant (const)
// Types: Number (int), big unsigned number (b_int), string (str)
//
// There should be at least one field with 'id' modifier.
// Only one field should have 'id' modifier.

class InMemoryFileParser
{
private:
    size_t format_version;

    const char *const &filename;
    Columns columns;

    size_t id_field_index;

    std::fstream open(const std::ios_base::openmode mode) const
    {
        std::fstream file;

        file.open(this->filename, mode);

#ifdef DEBUG
        debug_puts(filename, "InMemoryFileParser.open");
#endif

        if (!file.is_open()) {
            throw std::ios::failure("Could not open file");
        }

        return file;
    }

    // Reads first line of a file and updates InMemoryFileParser format version
    void update_version(std::fstream &file)
    {
        // Check first line of the file.
        std::string temp;
        std::getline(file, temp);

        // Check for magic string
        for (int i = 0; i < 3; ++i) {
            if (temp[i] != MAGIC[i]) {
                std::string failstring = "Database file format is not "
                                         "correct: File is not a tdb database";

                throw std::logic_error(failstring);
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

        size_t version = cm_parsell(version_string);

        if (version == COMMON_INVALID_NUMBERLL) {
            throw std::logic_error("Database format is not "
                                   "correct: Invalid version number");
        }

        if (version > PARSER_VERSION) {
            throw std::logic_error("Database format is not "
                                   "supported: Version is too new");
        };

#ifdef DEBUG
        debug_puts(version, "InMemoryFileParser.update_version");
#endif

        this->format_version = version;
    }

    // Should parse second line of the database file.
    // Line should look like this:
    // `|[modifier] <type> <name>|...`
    // Returns column names.
    std::vector<std::string> update_types(std::fstream &file)
    {
        Columns fields;
        std::vector<std::string> names;

        int field_n = 0;
        size_t pos  = 1;

        size_t id_pos = 0;

        char c = file.get();

        if (c != '|') {
            throw std::logic_error("update_types() first char is not |");
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
                        type |= (int)FINT;
                        set_type = true;
                    }
                    else if (buf == "b_int" && !set_type) {
                        type |= (int)FB_INT;
                        set_type = true;
                    }
                    else if (buf == "str" && !set_type) {
                        type |= (int)FSTR;
                        set_type = true;
                    }
                    else if (buf == "const") {
                        type |= (int)FCONST;
                    }
                    else if (buf == "id") {
                        if (set_id) {
                            std::string failstring =
                                "Format error: ID is already set at 2:" +
                                std::to_string(pos) +
                                ", previous ID at 2:" + std::to_string(id_pos);

                            throw std::logic_error(failstring);
                        }

                        type |= (int)FID;

                        this->id_field_index = field_n;

                        id_pos = pos;
                        set_id = true;
                    }
                    else if (!set_type) {
                        std::string failstring =
                            "Format error: Unknown type modifier at 2:" +
                            std::to_string(pos);
                        throw std::logic_error(failstring);
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
                throw std::logic_error(failstring);
            }

            names.push_back(buf);

            fields.types.push_back(type);
            ++field_n;

            c = file.get();
            ++pos;
        }

#ifdef DEBUG
        debug_putv(fields.types, "InMemoryFileParser.update_types types");
#endif

#ifdef DEBUG
        debug_putv(names, "InMemoryFileParser.update_types names");
#endif

        fields.size   = field_n;
        this->columns = fields;

        return names;
    }

    // Read file from disk into memory.
    std::vector<ParserColumn *> deserealize(std::fstream &file,
                                            std::vector<std::string> &names)
    {
        switch (this->format_version) {
            case 1: {
                return FormatOne::deserealize(file, this->columns, names);
            } break;

            default:
                throw std::logic_error("Unreachable");
        }
    }

    // Save vector of students from memory into file.
    void serialize(std::fstream &file,
                   const std::vector<ParserColumn *> &columns) const
    {
        switch (this->format_version) {
            case 1: {
                return FormatOne::serialize(file, columns);
            } break;

            default:
                throw std::logic_error("Unreachable");
        }
    }

public:
    InMemoryFileParser(const char *const &filename) : filename(filename)
    {
        this->format_version = PARSER_VERSION;
    }

    ~InMemoryFileParser()
    {
        delete this->filename;
    }

    size_t get_version() const
    {
        return this->format_version;
    }

    size_t get_id_column_index() const
    {
        return this->id_field_index;
    }

    // Return true if file exists, false if it doesn't.
    bool exists() const
    {
        std::fstream file;

        file.open(this->filename);

#ifdef DEBUG
        debug_puts(file.good(), "InMemoryFileParser.exists");
#endif

        return file.good();
    }

    // Return true if file exists, false if it doesn't.
    // Creates file if it didn't exist.
    bool exists_or_create() const
    {
        if (!this->exists()) {
            std::fstream file;
            file.open(this->filename, std::ios::out);

            if (!file.good()) {
                throw std::ios::failure("Could not create file");
            }

            // FormatOne::write_header(file);

            return false;
        }

        return true;
    }

    std::vector<ParserColumn *> read_file()
    {
        std::fstream file;

        file = this->open(std::ios::in | std::ios::out | std::ios::binary);

        this->update_version(file);

        std::vector<std::string> names = this->update_types(file);

        std::vector<ParserColumn *> columns = this->deserealize(file, names);

        file.close();
        return columns;
    }

    void write_file(const std::vector<ParserColumn *> &columns) const
    {
        if (!this->exists()) {
            throw std::runtime_error("File does not exist");
        }

        std::fstream file =
            this->open(std::ios::out | std::ios::trunc | std::ios::binary);

        this->serialize(file, columns);
        file.close();
    }
};
