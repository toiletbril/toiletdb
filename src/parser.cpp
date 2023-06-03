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
// Types: Number (int), Unsigned number (uint), string (str)
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

        TDB_DEBUGS(self->filename, "InMemoryFileParser.open");

        if (!file.is_open()) {
            throw std::ios::failure("In ToiletDB, In InMemoryFileParser.open(), could not open file");
        }

        return file;
    }

    // Reads first line of a file and updates InMemoryFileParser format version
    static void update_version(InMemoryFileParser *self, std::fstream &file)
    {
        switch (self->format_version) {
            case 1: {
                self->format_version = FormatOne::read_version(file);
            } break;

            default:
                throw ParsingError("In ToiletDB, InMemoryFileParser.read_version(), invalid format version");
        }
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
                throw ParsingError("In ToiletDB, InMemoryFileParser.read_types(), invalid format version");
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
                throw ParsingError("In ToiletDB, InMemoryFileParser.deserialize(), invalid format version");
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
                throw ParsingError("In ToiletDB, InMemoryFileParser.serialize(), invalid format version");
        }
    }
};

InMemoryFileParser::InMemoryFileParser(const std::string &filename) :
    filename(filename)
{
    this->format_version = TOILETDB_PARSER_FORMAT_VERSION;
}

InMemoryFileParser::~InMemoryFileParser()
{}

const size_t &InMemoryFileParser::get_version() const
{
    TDB_DEBUGS(this->columns.id_field_index, "InMemoryFileParser.get_version");
    return this->format_version;
}

const size_t &InMemoryFileParser::get_id_column_index() const
{
    TDB_DEBUGS(this->columns.id_field_index, "InMemoryFileParser.get_id_column_index");
    return this->columns.id_field_index;
}

// Return true if file exists, false if it doesn't.
bool InMemoryFileParser::exists() const
{
    std::fstream file;

    file.open(this->filename);

    TDB_DEBUGS(file.is_open(), "InMemoryFileParser.exists");

    return file.is_open();
}

// Return true if file exists, false if it doesn't.
// Creates file if it didn't exist.
bool InMemoryFileParser::exists_or_create() const
{
    if (!this->exists()) {
        std::fstream file;
        file.open(this->filename, std::ios::out);

        if (!file.good()) {
            throw std::ios::failure("In ToiletDB, InMemoryFileParser.exists_or_create(), could not create a file");
        }

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
        throw std::runtime_error("In ToiletDB, InMemoryFileParser.write_file(), file does not exist");
    }

    std::fstream file =
        Private::open(this, std::ios::out | std::ios::trunc | std::ios::binary);

    Private::serialize(this, file, columns);

    file.close();
}

const std::vector<int> &InMemoryFileParser::types() const
{
    return this->columns.types;
}

const std::vector<std::string> &InMemoryFileParser::names() const
{
    return this->columns.names;
}

} // namespace toiletdb
