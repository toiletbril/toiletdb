#include "table.hpp"

namespace toiletdb {

struct InMemoryTable::Private
{
    std::vector<unsigned long long> index;
    std::vector<Column *> columns;
    InMemoryFileParser *parser;

    static void erase(InMemoryTable *self, const size_t pos)
    {
        // Erase data in all columns
        size_t len = self->columns.size();

        for (size_t i = 0; i < len; ++i) {
            self->columns[i]->erase(pos);
        }
    }

    // Reads column marked as 'id',
    // updates index with a sorted list that maps position to ID.
    static void update_index(InMemoryTable *self)
    {
        std::vector<unsigned long long> id_column =
            *(static_cast<std::vector<unsigned long long> *>(
                self->columns[self->parser->get_id_column_index()]
                    ->get_data()));

        std::vector<unsigned long long> index;
        index.resize(id_column.size());

        std::iota(index.begin(), index.end(), 0);

        std::stable_sort(index.begin(), index.end(),
                         [&id_column](size_t a, size_t b) {
                             return id_column[a] < id_column[b];
                         });

#ifndef NDEBUG
        debug_putv(id_column, "ids");
        debug_putv(index, "index");
#endif

        self->index = index;
    }
};

// TODO: This does not create a file.
// IDs from loaded file will be indexed to be used for binary search.
InMemoryTable::InMemoryTable(const char *const &filename)
{
    this->parser = new InMemoryFileParser(filename);

    if (!this->parser->exists()) {
        throw std::runtime_error(
            "In InMemoryTable constructor, there is no such file.");
    }

    this->columns = this->parser->read_file();

    Private::update_index(this);
}

InMemoryTable::~InMemoryTable()
{
    delete this->parser;
}

const std::vector<Column *> &InMemoryTable::get_all() const
{
    return this->columns;
}

void InMemoryTable::reread_file()
{
    this->columns = this->parser->read_file();
}

void InMemoryTable::write_file() const
{
    this->parser->write_file(this->columns);
}

// Search methods return index of the element in the vector.
// If element is not found, return TDB_NOT_FOUND.

// Binary search by id.
size_t InMemoryTable::search(const size_t &id) const
{
    // NOTE: Long instead of size_t.
    long L = 0;
    long R = this->index.size();
    long m;

    std::vector<unsigned long long> *ids =
        static_cast<std::vector<unsigned long long> *>(
            this->columns[this->parser->get_id_column_index()]->get_data());

#ifndef NDEBUG
    debug_putv(*ids, "ids");
    debug_putv(this->index, "index");
#endif

    while (L <= R) {
        m = (L + R) / 2;

        if ((*ids)[this->index[m]] < id) {
            L = m + 1;
        }
        else if ((*ids)[this->index[m]] > id) {
            // If this would become unsigned,
            // here needs to be a test that breaks loop when R wraps.
            R = m - 1;
        }
        else {
            return this->index[m];
        }
    }

    return TDB_NOT_FOUND;
}

// Naive search any column by comparing strings.
std::vector<size_t> InMemoryTable::search(const std::string &name,
                                          std::string &query) const
{
    size_t pos = 0;
    std::vector<size_t> result;

    size_t column_index = this->search_column_index(name);

    if (column_index == TDB_NOT_FOUND) {
        std::string failstring =
            "In InMemoryTable.search(), Field '" + name + "' does not exist";
        throw std::logic_error(failstring);
    }

    int type = this->columns[column_index]->get_type();

    void *data = this->columns[column_index]->get_data();

    for (size_t i = 0; i < this->total_rows(); ++i) {
        std::string value;
        switch (type & TDB_TMASK) {
            case T_INT: {
                value = std::to_string(
                    (*(static_cast<std::vector<int> *>(data)))[i]);
            } break;
            case T_B_INT: {
                value = std::to_string((*(
                    static_cast<std::vector<unsigned long long> *>(data)))[i]);
                break;
            }
            case T_STR: {
                value = (*(static_cast<std::vector<std::string> *>(data)))[i];
                break;
            }
        }

        if (value.rfind(query, 0) == 0) {
            result.push_back(pos);
        }
        ++pos;
    }

    return result;
}

// To get types, use column_types().
// This returns void pointers to values from every column on a single row.
std::vector<void *> InMemoryTable::get_row(const size_t &pos)
{
    std::vector<void *> result;

    if (pos > this->total_rows()) {
        throw std::logic_error(
            "In InMemoryTable.get_row(), pos is larger than data size");
    }

    for (Column *c : this->columns) {
        // This is kinda unbearable
        switch (c->get_type() & TDB_TMASK) {
            case T_INT: {
                result.push_back(static_cast<void *>(
                    &(*static_cast<std::vector<int> *>((*c).get_data()))[pos]));
            } break;

            case T_B_INT: {
                result.push_back(static_cast<void *>(
                    &(*static_cast<std::vector<unsigned long long> *>(
                        (*c).get_data()))[pos]));
            } break;

            case T_STR: {
                result.push_back(static_cast<void *>(
                    &(*static_cast<std::vector<std::string> *>(
                        (*c).get_data()))[pos]));
            } break;
        }
    }

    return result;
}

// Returns 0 on success.
// Errors numbers:
// 1 - Args vector is too big/small.
// 2 - Argument of type 'int' is found to be
//     not convertible to int.
// 3 - Argument of type 'b_int' is found to be
//     not convertible to unsigned long long.
int InMemoryTable::add(std::vector<std::string> &args)
{
    std::vector<int> types = this->get_column_types();

    // Column count, ignoring ID.
    if (args.size() != this->get_column_count() - 1) {
        return 1;
    }

    std::vector<std::string>::iterator it = args.begin();

    // TODO:
    // This is just typechecking.
    // Surely this can be compressed (Clueless)
    for (size_t i = 1; i < this->get_column_count(); ++i) {
        if (types[i] & T_ID) {
            continue;
        }

        switch (types[i] & TDB_TMASK) {
            case T_INT: {
                int value = cm_parsei(*it++);

                if (value == TDB_INVALID_I) {
                    return 2;
                }
            } break;

            case T_B_INT: {
                size_t value = cm_parsell(*it++);

                if (value == TDB_INVALID_ULL) {
                    return 3;
                }
            } break;

            default: {
                ++it;
            }
        }
    }

    it = args.begin();

    // TODO: Doesn't look like a transaction to me
    for (size_t i = 0; i < this->get_column_count(); ++i) {
        if (types[i] & T_ID) {
            size_t new_id = this->get_next_id();
            this->columns[i]->add(static_cast<void *>(&new_id));
        }
        else if (types[i] & T_INT) {
            int value = cm_parsei(*it++);
            this->columns[i]->add(static_cast<void *>(&value));
        }
        else if (types[i] & T_B_INT) {
            unsigned long long value = cm_parsell(*it++);
            this->columns[i]->add(static_cast<void *>(&value));
        }
        else if (types[i] & T_STR) {
            this->columns[i]->add(static_cast<void *>(&(*it++)));
        }
    }

    return 0;
}

bool InMemoryTable::erase_id(const size_t &id)
{
    size_t result = this->search(id);

    if (result != TDB_NOT_FOUND) {
        Private::erase(this, result);
        return true;
    }

    return false;
}

void InMemoryTable::clear()
{
    for (Column *c : this->columns) {
        c->clear();
    }
}

size_t InMemoryTable::get_column_count() const
{
    return this->columns.size();
}

const std::vector<std::string> InMemoryTable::get_column_names() const
{
    std::vector<std::string> names;

    for (Column *c : this->columns) {
        names.push_back(c->get_name());
    }

    return names;
}

size_t InMemoryTable::search_column_index(const std::string &name) const
{
    std::vector<size_t> result;

    size_t column_index = TDB_INVALID_ULL;
    size_t j            = 0;

    for (const Column *col : this->columns) {
        if (col->get_name() == name) {
            column_index = j;
            break;
        }
        ++j;
    }

    if (column_index == TDB_INVALID_ULL) {
        return TDB_NOT_FOUND;
    }

    return column_index;
}

const std::vector<int> InMemoryTable::get_column_types() const
{
    std::vector<int> types;
    for (Column *c : this->columns) {
        types.push_back(c->get_type());
    }

    return types;
}

// Returns total amount of rows.
size_t InMemoryTable::total_rows() const
{
    if (!this->columns[0]) {
        throw std::logic_error("In InMemoryTable.size(), there is no columns");
    }

    return this->columns[0]->size();
}

size_t InMemoryTable::get_next_id() const
{
    while (true) {
        size_t next_id = this->total_rows();
        if (this->search(next_id) == TDB_NOT_FOUND) {
            return this->total_rows();
        }
    }
}

} // namespace toiletdb
