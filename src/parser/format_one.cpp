#pragma once

#include <fstream>
#include <vector>

#include "../common.cpp"
#include "../debug.cpp"

#include "parser.cpp"
#include "types.cpp"

struct FormatOne
{
    static std::vector<ParserColumn *>
    deserealize(std::fstream &file, Columns &columns,
                std::vector<std::string> &names)
    {
        // Allocate memory for each field.

        std::vector<ParserColumn *> parsed_columns;
        parsed_columns.reserve(columns.size);

        if (names.size() != columns.size)
        {
            std::string failstring =
                "In FormatOne.deserialize(), names.size() (" +
                std::to_string(names.size()) +
                ") is not equal to columns.size (" +
                std::to_string(columns.size) + ")";

            throw std::logic_error(failstring);
        }

        for (size_t i = 0; i < columns.size; ++i)
        {
            int type = columns.types[i];

            if (type & FINT)
            {
                ParserColumn *v = new ParserColumnInt(names[i], type);
                parsed_columns.push_back(v);
            }
            else if (type & FB_INT)
            {
                ParserColumn *v = new ParserColumnB_Int(names[i], type);
                parsed_columns.push_back(v);
            }
            else if (type & FSTR)
            {
                ParserColumn *v = new ParserColumnStr(names[i], type);
                parsed_columns.push_back(v);
            }
        }

        std::string temp;

        // Data starts from third line.
        // First line is magic, second is types.
        size_t line = 3;
        size_t pos  = 1;
#ifdef DEBUG
        bool debug_crlf = false;
#endif
        int c = file.get();

        while (c != EOF)
        {
            if (c != '|')
            {
                std::string failstring = "Database file format is not "
                                         "correct: Invalid delimiter at line " +
                                         std::to_string(line) + ":" +
                                         std::to_string(pos);

                throw std::logic_error(failstring);
            }
            else
            {
                c = file.get();
            }

            std::vector<std::string> fields;
            fields.reserve(columns.size);
            size_t field = 0;

            while (c != '\n')
            {
                if (c == '\r')
                {
#ifdef DEBUG
                    if (!debug_crlf)
                    {
                        debug_puts("CRLF detected",
                                   "InMemoryFileParser.deserialize");
                        debug_crlf = true;
                    }
#endif
                    c = file.get();
                    continue;
                }

                if (field > columns.size)
                {
                    std::string failstring = "Database file format is not "
                                             "correct: Extra field at line " +
                                             std::to_string(line) + ":" +
                                             std::to_string(pos + 1);

                    throw std::logic_error(failstring);
                }

                if (c == '|')
                {
                    fields.push_back(temp);
                    temp.clear();
                    ++field;
                }
                else
                {
                    temp += c;
                }

                c = file.get();
                ++pos;
            }

            if (field != columns.size)
            {
                std::string failstring =
                    "Database file format is not "
                    "correct: Invalid number of fields (" +
                    std::to_string(columns.size) + " required, actual " +
                    std::to_string(field) + ") at line " +
                    std::to_string(line) + ":" + std::to_string(pos);

                throw std::logic_error(failstring);
            }

#ifdef DEBUG
            debug_putv(fields, "InMemoryFileParser.deserealize");
#endif

            for (size_t i = 0; i < columns.size; ++i)
            {
                if (columns.types[i] & FID)
                {
                    if (!(columns.types[i] & FB_INT))
                    {
                        std::string failstring = "Database file format is not "
                                                 "correct: Field with modifier "
                                                 "'id' is not of type 'b_int', "
                                                 "line " +
                                                 std::to_string(line) +
                                                 ", field " +
                                                 std::to_string(i + 1);

                        throw std::logic_error(failstring);
                    }
                }
                if (columns.types[i] & FINT)
                {
                    size_t num = cm_parsei(fields[i]);

                    if (num == COMMON_INVALID_NUMBERI)
                    {
                        std::string failstring =
                            "Database file format is not "
                            "correct: Field of type 'int' is not a number, "
                            "line " +
                            std::to_string(line) + ", field " +
                            std::to_string(i + 1);

                        throw std::logic_error(failstring);
                    }

                    static_cast<std::vector<int> *>(
                        parsed_columns[i]->get_data())
                        ->push_back(num);
                }
                else if (columns.types[i] & FB_INT)
                {
                    size_t num = cm_parsell(fields[i]);

                    if (num == COMMON_INVALID_NUMBERLL)
                    {
                        std::string failstring =
                            "Database file format is not "
                            "correct: Field of type 'b_int' is not a number, "
                            "line " +
                            std::to_string(line) + ", field " +
                            std::to_string(i + 1);

                        throw std::logic_error(failstring);
                    }

                    static_cast<std::vector<unsigned long long> *>(
                        parsed_columns[i]->get_data())
                        ->push_back(num);
                }

                else if (columns.types[i] & FSTR)
                {
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

    static void write_header(std::fstream &file,
                             const std::vector<ParserColumn *> &data)
    {
        std::string header = "tdb1\n";

        for (const ParserColumn *c : data)
        {
            header += '|';

            int modifiers = c->get_type();

            // Write modifiers
            if (modifiers & FID)
            {
                header += "id ";
            }

            if (modifiers & FCONST)
            {
                header += "const ";
            }

            // Write type
            switch (modifiers & PARSER_TYPE_MASK)
            {
                case FINT: {
                    header += "int ";
                }
                break;

                case FB_INT: {
                    header += "b_int ";
                }
                break;

                case FSTR: {
                    header += "str ";
                }
                break;
            }

            // Write name. No space at the end.
            header += c->get_name();
        }
        header += '|';

#ifdef DEBUG
        debug_puts(header, "InMemoryFileParser.write_header");
#endif

        file << header << std::endl;
    }

    // Save vector of students from memory into file.
    static void serialize(std::fstream &file,
                          const std::vector<ParserColumn *> &data)
    {
        FormatOne::write_header(file, data);

        size_t column_count = data.size();

        if (!data[0])
        {
            std::logic_error(
                "In FormatOne.serialize(), there is no elements in data");
        }

        size_t row_count = data[0]->size();

        std::vector<int> types;
        types.reserve(column_count);

        for (const ParserColumn *c : data)
        {
            types.push_back(c->get_type());
        }

        for (size_t row = 0; row < row_count; ++row)
        {
            for (size_t col = 0; col < column_count; ++col)
            {
                file << '|';
                ParserColumn *c = data[col];
                void *item      = c->get(row);

                switch (c->get_type() & PARSER_TYPE_MASK)
                {
                    case FINT: {
                        file << *(static_cast<int *>(item));
                    }
                    break;

                    case FB_INT: {
                        file << *(static_cast<unsigned long long *>(item));
                    }
                    break;

                    case FSTR: {
                        file << *(static_cast<std::string *>(item));
                    }
                    break;
                }
            }
            file << "|\n";
        }
#ifdef DEBUG
        debug_puts(row_count, "InMemoryFileParser.serialize rows saved");
#endif
    }
};
