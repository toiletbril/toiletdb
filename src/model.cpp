#pragma once

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <numeric>
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
    std::vector<unsigned long long> indexes;
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

    // Creates sorted index of IDs for binary search.
    void index()
    {
        std::vector<unsigned long long> id_column =
            *(static_cast<std::vector<unsigned long long> *>(
                this->columns[this->parser->get_id_column_index()]
                    ->get_data()));

        std::vector<unsigned long long> index;
        index.resize(id_column.size());

        std::iota(index.begin(), index.end(), 0);

        std::stable_sort(index.begin(), index.end(),
                         [&id_column](size_t a, size_t b) {
                             return id_column[a] < id_column[b];
                         });

#ifdef DEBUG
        debug_putv(id_column, "ids");
        debug_putv(index, "index");
#endif

        this->indexes = index;
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
            throw std::runtime_error(
                "In InMemoryModel constructor, there is no such file.");
        }

        this->columns = this->parser->read_file();

        this->index();
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

    // Binary search by id.
    size_t search(const size_t &id) const
    {
        // NOTE: long instead of size_t
        long L = 0;
        long R = this->indexes.size();
        long m = std::floor((L + R) / 2);

        std::vector<unsigned long long> *ids =
            static_cast<std::vector<unsigned long long> *>(
                this->columns[this->parser->get_id_column_index()]->get_data());

#ifdef DEBUG
        debug_putv(*ids, "ids");
        debug_putv(this->indexes, "index");
#endif

        while (L <= R)
        {
            m = (L + R) / 2;

            if ((*ids)[this->indexes[m]] < id)
            {
                L = m + 1;
            }
            else if ((*ids)[this->indexes[m]] > id)
            {
                // If this would become unsigned,
                // here needs to be a test that breaks loop when R wraps
                R = m - 1;
            }
            else
            {
                return this->indexes[m];
            }
        }

        return MODEL_NOT_FOUND;
    }

    // Search any column by comparing strings.
    std::vector<size_t> search(const std::string &name,
                               std::string &query) const
    {
        size_t pos = 0;
        std::vector<size_t> result;

        size_t column_index = this->column_index(name);

        if (column_index == MODEL_NOT_FOUND)
        {
            std::string failstring = "In InMemoryModel.search(), Field '" +
                                     name + "' does not exist";
            throw std::logic_error(failstring);
        }

        int type = this->columns[column_index]->get_type();

        void *data = this->columns[column_index]->get_data();

        for (size_t i = 0; i < this->size(); ++i)
        {
            std::string value;
            switch (type & PARSER_TYPE_MASK)
            {
                case FINT: {
                    value = std::to_string(
                        (*(static_cast<std::vector<int> *>(data)))[i]);
                }
                break;
                case FB_INT: {
                    value = std::to_string(
                        (*(static_cast<std::vector<unsigned long long> *>(
                            data)))[i]);
                    break;
                }
                case FSTR: {
                    value =
                        (*(static_cast<std::vector<std::string> *>(data)))[i];
                    break;
                }
            }

            if (value.rfind(query, 0) == 0)
            {
                result.push_back(pos);
            }
            ++pos;
        }

        return result;
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

    bool add(std::vector<std::string> &args)
    {
        std::vector<int> types = this->column_types();

        // Ignoring ID
        if (args.size() != this->column_count() - 1)
        {
            return false;
        }

        std::vector<std::string>::iterator it = args.begin();

        // TODO:
        // This is just typechecking.
        // Surely this can be compressed (Clueless)
        for (size_t i = 1; i < this->column_count(); ++i)
        {
            if (types[i] & FID)
            {
                continue;
            }

            switch (types[i] & PARSER_TYPE_MASK)
            {
                case FINT: {
                    int value = cm_parsei(*it++);

                    if (value == COMMON_INVALID_NUMBERI)
                    {
                        return false;
                    }
                }
                break;

                case FB_INT: {
                    size_t value = cm_parsell(*it++);

                    if (value == COMMON_INVALID_NUMBERLL)
                    {
                        return false;
                    }
                }
                break;

                default: {
                    ++it;
                }
            }
        }

        it = args.begin();

        // TODO: Doesn't look like a transaction to me
        for (size_t i = 0; i < this->column_count(); ++i)
        {
            if (types[i] & FID)
            {
                size_t new_id = this->get_next_id();
                this->columns[i]->add(static_cast<void *>(&new_id));
            }
            else if (types[i] & FINT)
            {
                int value = cm_parsei(*it++);
                this->columns[i]->add(static_cast<void *>(&value));
            }
            else if (types[i] & FB_INT)
            {
                unsigned long long value = cm_parsell(*it++);
                this->columns[i]->add(static_cast<void *>(&value));
            }
            else if (types[i] & FSTR)
            {
                this->columns[i]->add(static_cast<void *>(&(*it++)));
            }
        }
        return true;
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
        for (ParserColumn *c : this->columns)
        {
            c->clear();
        }
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

    size_t column_index(const std::string &name) const
    {
        std::vector<size_t> result;

        size_t column_index = COMMON_INVALID_NUMBERLL;
        size_t j            = 0;

        for (const ParserColumn *col : this->columns)
        {
            if (col->get_name() == name)
            {
                column_index = j;
                break;
            }
            ++j;
        }

        if (column_index == COMMON_INVALID_NUMBERLL)
        {
            return MODEL_NOT_FOUND;
        }

        return column_index;
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
