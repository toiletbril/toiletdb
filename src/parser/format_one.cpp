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
            if (columns.types[i] & FINT)
            {
                ParserColumn *v = new ParserColumnInt(names[i]);
                parsed_columns.push_back(v);
            }

            if (columns.types[i] & FB_INT)
            {
                ParserColumn *v = new ParserColumnB_Int(names[i]);
                parsed_columns.push_back(v);
            }

            if (columns.types[i] & FSTR)
            {
                ParserColumn *v = new ParserColumnStr(names[i]);
                parsed_columns.push_back(v);
            }
        }

        std::string temp;

        size_t line = 1;
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
                if (columns.types[i] & FINT)
                {
                    size_t num = cm_parsei(fields[i]);
                    if (num == COMMON_INVALID_NUMBERLL)
                    {
                        std::string failstring =
                            "Database file format is not "
                            "correct: b_int field is not a number, "
                            "line " +
                            std::to_string(line) + ", field " +
                            std::to_string(i);

                        throw std::logic_error(failstring);
                    }

                    static_cast<std::vector<int> *>(
                        parsed_columns[i]->get_data())
                        ->push_back(num);
                }

                else if (columns.types[i] & FB_INT)
                {
                    size_t num = cm_parsell(fields[i]);
                    if (num == COMMON_INVALID_NUMBERI)
                    {
                        std::string failstring =
                            "Database file format is not "
                            "correct: b_int field is not a number, "
                            "line " +
                            std::to_string(line) + ", field " +
                            std::to_string(i);

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

    // Save vector of students from memory into file.
    static void serialize(std::fstream &file, std::vector<ParserColumn *> data)
    {
        (void)(file);
        (void)(data);

        //         for (const Student &student : students)
        //         {
        // #ifdef DEBUG
        //             debug_puts(student, "InMemoryFileParser.serealize");
        // #endif

        //             file << '|' << student.get_id() << '|' << student.name <<
        //             '|'
        //                  << student.surname << '|' << student.group << '|'
        //                  << student.record_book << '|' << "\n";
        //         }

        //         file.flush();
    }
};
