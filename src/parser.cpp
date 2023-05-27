#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#include "parser.hpp"

#define MAGIC "tdb"

// File format:
// 1    tdb002
// 2    |[modifier] <type> <name>|...
//
// Modifiers: ID (id), constant (const)
// Types: Number (int), big unsigned number (b_int), string (str)
//
// There should be at least one field with 'id' modifier.
// Only one field should have 'id' modifier.

namespace toiletdb {

struct InMemoryFileParser::Private
{
    static std::fstream open(const InMemoryFileParser *self,
                             const std::ios_base::openmode mode)
    {
        std::fstream file;

        file.open(self->filename, mode);

#ifndef NDEBUG
        debug_puts(self->filename, "InMemoryFileParser.open");
#endif

        if (!file.is_open()) {
            throw std::ios::failure("Could not open file");
        }

        return file;
    }

    // Reads first line of a file and updates InMemoryFileParser format version
    static void update_version(InMemoryFileParser *self, std::fstream &file)
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

        if (version == TDB_INVALID_ULL) {
            throw std::logic_error("Database format is not "
                                   "correct: Invalid version number");
        }

        if (version > TDB_INVALID_I) {
            throw std::logic_error("Database format is not "
                                   "supported: Version is too new");
        };

#ifdef DEBUG
        debug_puts(version, "InMemoryFileParser.update_version");
#endif

        self->format_version = version;
    }

    // Should parse second line of the database file.
    // Line should look like this:
    // `|[modifier] <type> <name>|...`
    // Updates this->columns.
    static void read_types(InMemoryFileParser *self, std::fstream &file)
    {
        switch (self->format_version) {
            case 1: {
                self->columns = FormatOne::read_types(file);
            } break;

            default:
                throw std::logic_error("Unreachable");
        }
    }

    // Read file from disk into memory.
    static std::vector<Column *> deserealize(InMemoryFileParser *self,
                                             std::fstream &file,
                                             std::vector<std::string> &names)
    {
        switch (self->format_version) {
            case 1: {
                return FormatOne::deserealize(file, self->columns, names);
            } break;

            default:
                throw std::logic_error("Unreachable");
        }
    }

    static void serialize(const InMemoryFileParser *self,
                          std::fstream &file,
                          const std::vector<Column *> &columns)
    {
        switch (self->format_version) {
            case 1: {
                return FormatOne::serialize(file, columns);
            } break;

            default:
                throw std::logic_error("Unreachable");
        }
    }
};

InMemoryFileParser::InMemoryFileParser(const char *const &filename) :
    filename(filename)
{
    this->format_version = TOILETDB_PARSER_VERSION;
}

InMemoryFileParser::~InMemoryFileParser()
{
    delete this->filename;
}

size_t InMemoryFileParser::get_version() const
{
    return this->format_version;
}

size_t InMemoryFileParser::get_id_column_index() const
{
    return this->columns.id_field_index;
}

// Return true if file exists, false if it doesn't.
bool InMemoryFileParser::exists() const
{
    std::fstream file;

    file.open(this->filename);

#ifndef NDEBUG
    debug_puts(file.good(), "InMemoryFileParser.exists");
#endif

    return file.good();
}

// Return true if file exists, false if it doesn't.
// Creates file if it didn't exist.
bool InMemoryFileParser::exists_or_create() const
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

std::vector<Column *> InMemoryFileParser::read_file()
{
    std::fstream file;

    file = Private::open(this, std::ios::in | std::ios::out | std::ios::binary);

    Private::update_version(this, file);
    Private::read_types(this, file);

    std::vector<std::string> names = this->columns.names;

    std::vector<Column *> columns = Private::deserealize(this, file, names);

    file.close();
    return columns;
}

void InMemoryFileParser::write_file(const std::vector<Column *> &columns) const
{
    if (!this->exists()) {
        throw std::runtime_error("File does not exist");
    }

    std::fstream file =
        Private::open(this, std::ios::out | std::ios::trunc | std::ios::binary);

    Private::serialize(this, file, columns);
    file.close();
}

} // namespace toiletdb
