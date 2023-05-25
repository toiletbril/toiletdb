#pragma once

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <vector>

#ifdef DEBUG
    #include "debug.cpp"
#endif

#include "./parser/parser.cpp"
#include "./parser/types.cpp"

#define MODEL_NOT_FOUND (size_t)(-1)

class InMemoryModel
{
private:
    std::vector<ParserColumn *> columns;
    InMemoryFileParser *parser;

    void erase(const size_t pos)
    {
        // Erase data in all columns
        size_t len = this->columns.size();

        for (size_t i = 0; i < len; ++i)
        {
            this->columns[i]->erase(pos);
        }
    }

public:
    // This will create a file 'filename' and use it to store commits.
    // If file already exists, elements will be sorted by ID to suit binary
    // search.
    // Using previously unsorted file will result in it being reordered.
    InMemoryModel(const char *const &filename)
    {
        this->parser = new InMemoryFileParser(filename);

        if (!this->parser->exists())
        {
            throw std::logic_error("In InMemoryModel constructor, there is no such file.");
        }

        this->columns = this->parser->read_file();

        // TODO: sort
    }

    ~InMemoryModel()
    {
        delete this->parser;
    }

    const std::vector<ParserColumn *> &get_all() const
    {
        return this->columns;
    }

    // Clear student vector and parse the file again.
    void reread_file()
    {
        this->columns = this->parser->read_file();
    }

    // Serialize.
    void write_file() const
    {
        this->parser->write_file(this->columns);
    }

    // Search methods return index of the element in the vector.
    // If element is not found, returns MODEL_NOT_FOUND.

    // Search by id.
    size_t search(const size_t &id) const
    {
        size_t L = 0;
        size_t R = this->columns[0]->size();
        size_t m = -1;

        size_t id_col_index = this->parser->get_id_column_index();

        std::vector<unsigned long long> *ids =
            static_cast<std::vector<unsigned long long> *>(
                this->columns[id_col_index]->get_data());

        while (L <= R)
        {
            m = std::floor((L + R) / 2);

            if ((*ids)[m] < id)
            {
                L = m + 1;
            }
            else if ((*ids)[m] > id)
            {
                R = m - 1;
            }
            else
            {
                return m;
            }
        }

        return MODEL_NOT_FOUND;
    }

    // This returns a row of heterogenous types.
    // To get types, use column_types()
    std::vector<void *> get_row(const size_t &pos)
    {
        std::vector<void *> result;

        if (pos > this->size())
        {
            throw std::logic_error(
                "In InMemoryModel.get_row(), pos is larger than data size");
        }

        for (ParserColumn *c : this->columns)
        {
            // This is unbearable
            switch (c->get_type() & PARSER_TYPE_MASK)
            {
                case FINT: {
                    result.push_back(
                        static_cast<void *>(&(*static_cast<std::vector<int> *>(
                            (*c).get_data()))[pos]));
                }
                break;

                case FB_INT: {
                    result.push_back(static_cast<void *>(
                        &(*static_cast<std::vector<unsigned long long> *>(
                            (*c).get_data()))[pos]));
                }
                break;

                case FSTR: {
                    result.push_back(static_cast<void *>(
                        &(*static_cast<std::vector<std::string> *>(
                            (*c).get_data()))[pos]));
                }
                break;
            }
        }

        return result;
    }

    // Search str fields
    std::vector<size_t> search(const std::string &name,
                               std::string &query) const
    {
        size_t pos = 0;
        std::vector<size_t> result;

        size_t column_index = COMMON_INVALID_NUMBERLL;
        size_t j            = 0;

        for (const ParserColumn *col : this->columns)
        {
            if (col->get_name() == name)
            {
                column_index = j;
            }
            ++j;
        }

        if (column_index == COMMON_INVALID_NUMBERLL)
        {
            std::string failstring =
                "In InMemoryModel.search(), Invalid field '" + name + "'";
            throw std::logic_error(failstring);
        }

        int type = this->columns[column_index]->get_type();

        if (!(type & FSTR))
        {
            std::string failstring = "In InMemoryModel.search(), field '" +
                                     name + "' is not of str type";
            throw std::logic_error(failstring);
        }

        std::vector<std::string> *data =
            static_cast<std::vector<std::string> *>(
                this->columns[column_index]->get_data());

        size_t len = data->size();

        for (size_t i = 0; i < len; ++i)
        {
            std::string row_value = (*data)[i];
            cm_pstr_tolower(row_value);

            if (row_value.rfind(query, 0) == 0)
            {
                result.push_back(pos);
            }
            ++pos;
        }
        return result;
    }

    void add(...)
    {
        // #ifdef DEBUG
        //         debug_puts(s, "Model.add");
        // #endif
        //         if (this->search(s.get_id()) != MODEL_NOT_FOUND)
        //         {
        //             throw std::logic_error("An entry exists with same ID");
        //         }

        // this->students.push_back(s);
    }

    bool erase_id(const size_t id)
    {
        size_t result = this->search(id);

        if (result != MODEL_NOT_FOUND)
        {
            this->erase(result);
            return true;
        }

        return false;
    }

    void clear()
    {
        this->columns.clear();
    }

    size_t column_count() const
    {
        return this->columns.size();
    }

    const std::vector<std::string> column_names() const
    {
        std::vector<std::string> names;

        for (ParserColumn *c : this->columns)
        {
            names.push_back(c->get_name());
        }

        return names;
    }
    const std::vector<int> column_types() const
    {
        std::vector<int> types;
        for (ParserColumn *c : this->columns)
        {
            types.push_back(c->get_type());
        }

        return types;
    }

    size_t size() const
    {
        if (!this->columns[0])
        {
            throw std::logic_error(
                "In InMemoryModel.size(), there is no columns");
        }

        return this->columns[0]->size();
    }

    size_t get_next_id() const
    {
        return this->size();
    }
};
