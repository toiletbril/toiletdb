#include "table.hpp"

namespace toiletdb {

struct InMemoryTable::Private
{
    std::vector<unsigned long long> index;
    std::vector<Column *> columns;
    std::unique_ptr<InMemoryFileParser> parser;

    void erase(const size_t pos)
    {
        size_t len = this->columns.size();

        // Erase data from all columns in one row
        for (size_t i = 0; i < len; ++i) {
            this->columns[i]->erase(pos);
        }
    }

    // Reads column marked as 'id',
    // updates index with a sorted list that maps position to ID.
    void update_index()
    {
        std::vector<unsigned long long> id_column =
            TDB_GET_DATA(unsigned long long,
                         this->columns[this->parser->get_id_column_index()]);

        std::vector<unsigned long long> index;
        index.resize(id_column.size());

        std::iota(index.begin(), index.end(), 0);

        std::stable_sort(index.begin(), index.end(),
                         [&id_column](size_t a, size_t b) {
                             return id_column[a] < id_column[b];
                         });

        TOILET_DEBUGV(id_column, "ids");
        TOILET_DEBUGV(index, "index");

        this->index = index;
    }
};

InMemoryTable::InMemoryTable(const std::string &filename)
{
    // TODO: This does not create a file.
    // IDs from loaded file will be indexed to be used for binary search.

    this->private_ = std::make_unique<Private>();

    this->private_->parser = std::make_unique<InMemoryFileParser>(filename);

    if (!this->private_->parser->exists()) {
        throw std::runtime_error(
            "In InMemoryTable constructor, there is no such file.");
    }

    this->private_->columns = this->private_->parser->read_file();

    this->private_->update_index();
}

InMemoryTable::~InMemoryTable() {}

const std::vector<Column *> &InMemoryTable::get_all() const
{
    return this->private_->columns;
}

void InMemoryTable::reread_file()
{
    this->private_->columns = this->private_->parser->read_file();
}

void InMemoryTable::write_file() const
{
    this->private_->parser->write_file(this->private_->columns);
}

size_t InMemoryTable::search(const size_t &id) const
{
    // Search methods return index of the element in the vector.
    // If element is not found, return TDB_NOT_FOUND.
    // Binary search by id.

    // NOTE: Long instead of size_t.
    long L = 0;
    long R = this->private_->index.size();
    long m;

    std::vector<unsigned long long> id_column = TDB_GET_DATA(
        unsigned long long,
        this->private_->columns[this->private_->parser->get_id_column_index()]);

    TOILET_DEBUGV(id_column, "ids");
    TOILET_DEBUGV(this->private_->index, "index");

    while (L <= R) {
        m = (L + R) / 2;

        if (id_column[this->private_->index[m]] < id) {
            L = m + 1;
        }
        else if (id_column[this->private_->index[m]] > id) {
            // If this would become unsigned,
            // here needs to be a test that breaks loop when R wraps.
            R = m - 1;
        }
        else {
            return this->private_->index[m];
        }
    }

    return TDB_NOT_FOUND;
}

std::vector<size_t> InMemoryTable::search(const std::string &name,
                                          std::string &query) const
{
    // Naive search any column by comparing strings.
    size_t pos = 0;
    std::vector<size_t> result;

    size_t column_index = this->search_column_index(name);

    if (column_index == TDB_NOT_FOUND) {
        std::string failstring =
            "In ToiletDB, In InMemoryTable.search(), Field '" + name +
            "' does not exist";
        throw std::logic_error(failstring);
    }

    int type = this->private_->columns[column_index]->get_type();

    void *data = this->private_->columns[column_index]->get_data();

    for (size_t i = 0; i < this->get_row_count(); ++i) {
        std::string value;
        switch (TDB_TYPE(type)) {
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

std::vector<void *> InMemoryTable::get_row(const size_t &pos)
{
    // To get types, use column_types().
    // This returns void pointers to values from every column on a single row.

    std::vector<void *> result;

    if (pos > this->get_row_count()) {
        throw std::logic_error("In ToiletDB, In InMemoryTable.get_row(), pos "
                               "is larger than data size");
    }

    for (Column *c : this->private_->columns) {
        switch (TDB_TYPE(c->get_type())) {
            case T_INT: {
                result.push_back(
                    static_cast<void *>(&(TDB_GET_DATA(int, c))[pos]));
            } break;

            case T_B_INT: {
                result.push_back(static_cast<void *>(
                    &(TDB_GET_DATA(unsigned long long, c))[pos]));
            } break;

            case T_STR: {
                result.push_back(
                    static_cast<void *>(&(TDB_GET_DATA(std::string, c))[pos]));
            } break;
        }
    }

    return result;
}

int InMemoryTable::add(std::vector<std::string> &args)
{
    // Returns 0 on success.
    // Errors numbers:
    // 1 - Args vector is too big/small.
    // 2 - Argument of type 'int' is found to be
    //     not convertible to int.
    // 3 - Argument of type 'b_int' is found to be
    //     not convertible to unsigned long long.

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
        if (TDB_IS(types[i], T_ID)) {
            continue;
        }

        switch (TDB_TYPE(types[i])) {
            case T_INT: {
                int value = parse_int(*it++);

                if (value == TDB_INVALID_I) {
                    return 2;
                }
            } break;

            case T_B_INT: {
                size_t value = parse_long_long(*it++);

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
        if (TDB_IS(types[i], T_ID)) {
            size_t new_id = this->get_next_id();
            this->private_->columns[i]->add(static_cast<void *>(&new_id));
        }
        else if (TDB_IS(types[i], T_INT)) {
            int value = parse_int(*it++);
            this->private_->columns[i]->add(static_cast<void *>(&value));
        }
        else if (TDB_IS(types[i], T_B_INT)) {
            unsigned long long value = parse_long_long(*it++);
            this->private_->columns[i]->add(static_cast<void *>(&value));
        }
        else if (TDB_IS(types[i], T_STR)) {
            this->private_->columns[i]->add(static_cast<void *>(&(*it++)));
        }
    }

    return 0;
}

bool InMemoryTable::erase_id(const size_t &id)
{
    size_t result = this->search(id);

    if (result != TDB_NOT_FOUND) {
        this->private_->erase(result);
        return true;
    }

    return false;
}

void InMemoryTable::clear()
{
    for (Column *c : this->private_->columns) {
        c->clear();
    }
}

size_t InMemoryTable::get_column_count() const
{
    return this->private_->columns.size();
}

const std::vector<std::string> &InMemoryTable::get_column_names() const
{
    return this->private_->parser->names();
}

const std::string &InMemoryTable::get_column_name(const size_t &pos) const
{
    if (pos > this->get_column_count()) {
        throw std::logic_error(
            "In ToiletDB, In InMemoryTable.get_column_name(), pos "
            "is larger than column count");
    }

    return this->private_->parser->names()[pos];
}

size_t InMemoryTable::search_column_index(const std::string &name) const
{
    std::vector<size_t> result;

    size_t column_index = TDB_INVALID_ULL;
    size_t j            = 0;

    for (const Column *col : this->private_->columns) {
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

const std::vector<int> &InMemoryTable::get_column_types() const
{
    return this->private_->parser->types();
}

const int &InMemoryTable::get_column_type(const size_t &pos) const
{
    if (pos > this->get_column_count()) {
        throw std::logic_error(
            "In ToiletDB, In InMemoryTable.get_column_type(), pos "
            "is larger than column count");
    }

    return this->private_->parser->types()[pos];
}

size_t InMemoryTable::get_row_count() const
{
    if (!this->private_->columns[0]) {
        throw std::logic_error(
            "In ToiletDB, In InMemoryTable.size(), there is no columns");
    }

    return this->private_->columns[0]->size();
}

size_t InMemoryTable::get_next_id() const
{
    while (true) {
        size_t next_id = this->get_row_count();
        if (this->search(next_id) == TDB_NOT_FOUND) {
            return this->get_row_count();
        }
    }
}

} // namespace toiletdb
