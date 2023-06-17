#include "table.hpp"

namespace toiletdb {

struct InMemoryTable::Private
{
    std::vector<size_t> index;
    std::vector<ColumnBase *> columns;
    std::unique_ptr<InMemoryFileParser> parser;

    // Reads column marked as 'id',
    // updates index with a sorted list that maps position to ID.
    void update_index()
    {
        std::vector<size_t> id_column =
            static_cast<ColumnUint *>(this->columns[this->parser->id_column_index()])
            ->get_data();

        std::vector<size_t> index;
        index.resize(id_column.size());

        std::iota(index.begin(), index.end(), 0);

        std::stable_sort(index.begin(), index.end(),
                         [&id_column](size_t a, size_t b) {
                             return id_column[a] < id_column[b];
                         });

        TDB_DEBUGV(id_column, "ids");
        TDB_DEBUGV(index, "index");

        this->index = index;
    }
};

InMemoryTable::InMemoryTable(const std::string &filename)
{
    TDB_DEBUGS(filename, "InMemoryTable filename");

    this->private_ = std::make_unique<Private>();

    this->private_->parser = std::make_unique<InMemoryFileParser>(filename);

    if (!this->private_->parser->exists()) {
        throw std::runtime_error(
            "In InMemoryTable constructor, there is no such file.");
    }

    this->private_->columns = this->private_->parser->read_file();

    // IDs from loaded file will be indexed here to be used for binary search.
    this->private_->update_index();
}

InMemoryTable::~InMemoryTable()
{
    // Deallocate all columns on destruction.
    for (size_t i = 0; i < this->private_->columns.size(); ++i) {
        delete this->private_->columns[i];
    }
}

const std::vector<ColumnBase *> &InMemoryTable::get_all() const
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

    std::vector<size_t> id_column =
        static_cast<ColumnUint *>(this->private_->columns[this->private_->parser->id_column_index()])
        ->get_data();

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

    for (size_t i = 0; i < this->get_row_count(); ++i) {
        std::string value;
        switch (TDB_TYPE(type)) {
            case T_INT: {
                value = static_cast<ColumnInt *>(this->private_->columns[column_index])
                        ->get(i);
            } break;
            case T_UINT: {
                value = static_cast<ColumnUint *>(this->private_->columns[column_index])
                        ->get(i);
                break;
            }
            case T_STR: {
                value = static_cast<ColumnStr *>(this->private_->columns[column_index])
                        ->get(i);
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

    for (ColumnBase *c : this->private_->columns) {
        switch (TDB_TYPE(c->get_type())) {
            case T_INT: {
                result.push_back(static_cast<void *>(&(static_cast<ColumnInt *>(c))->get(pos)));
            } break;

            case T_UINT: {
                result.push_back(static_cast<void *>(&(static_cast<ColumnUint *>(c))->get(pos)));
            } break;

            case T_STR: {
                result.push_back(static_cast<void *>(&(static_cast<ColumnStr *>(c))->get(pos)));
            } break;
        }
    }

    return result;
}

int InMemoryTable::add_row(std::vector<std::string> &args)
{
    // NOTE: Do not pass ID column here.

    // Returns 0 on success.
    // Errors numbers:
    // 1 - Args vector is too big/small.
    // 2 - Argument of type 'int' is found to be
    //     not convertible to int.
    // 3 - Argument of type 'uint' is found to be
    //     not convertible to size_t.

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

        // Skip the ID column.
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

            case T_UINT: {
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
    // TODO: Removing rows that don't have a value in all columns?
    for (size_t i = 0; i < this->get_column_count(); ++i) {

        // NOTE: ID handling depends on get_next_id().
        if (TDB_IS(types[i], T_ID)) {
            size_t new_id = this->get_next_id();
            static_cast<ColumnUint *>(this->private_->columns[i])->add(new_id);
        }

        else if (TDB_IS(types[i], T_INT)) {
            int value = parse_int(*it++);
            static_cast<ColumnInt *>(this->private_->columns[i])->add(value);
        }

        else if (TDB_IS(types[i], T_UINT)) {
            size_t value = parse_long_long(*it++);
            static_cast<ColumnUint *>(this->private_->columns[i])->add(value);
        }

        else if (TDB_IS(types[i], T_STR)) {
            static_cast<ColumnStr *>(this->private_->columns[i])->add(*it++);
        }
    }

    return 0;
}

bool InMemoryTable::erase(const size_t &pos)
{
    size_t len = this->private_->columns.size();

    if (pos >= this->get_row_count()) {
        return false;
    }

    // Erase data from all columns in one row.
    for (size_t i = 0; i < len; ++i) {
        this->private_->columns[i]->erase(pos);
    }

    return true;
}

bool InMemoryTable::erase_id(const size_t &id)
{
    size_t result = this->search(id);

    if (result != TDB_NOT_FOUND) {
        this->erase(result);
        return true;
    }

    return false;
}

void InMemoryTable::clear()
{
    for (ColumnBase *c : this->private_->columns) {
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
    size_t i            = 0;

    for (const ColumnBase *col : this->private_->columns) {
        if (col->get_name() == name) {
            column_index = i;
            break;
        }
        ++i;
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
    // This should return valid and unique ID for a new row.
    // add() uses this.

    size_t i = 0;

    while (true) {
        size_t next_id = this->get_row_count() + i;
        if (this->search(next_id) == TDB_NOT_FOUND) {
            TDB_DEBUGS(next_id, "InMemoryTable.get_next_id");
            return next_id;
        }
        ++i;
    }
}

} // namespace toiletdb
