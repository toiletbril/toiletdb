#define TOILETDB_MAGIC "tdb"

#define TDB_TMASK 0b00000111
#define TDB_MMASK 0b00111000

#include "types.hpp"

namespace toiletdb {

struct TableInfo;

class ColumnBase;

template<typename T>
class Column;

ColumnInt::ColumnInt(std::string name, int type)
{
    TDB_DEBUGS(name, "ColumnInt name");
    TDB_DEBUGS(type, "ColumnInt type");

    this->name = name;
    this->type = type;
    this->data = new std::vector<int>;
}

ColumnInt::~ColumnInt()
{
    delete this->data;
}

const int &ColumnInt::get_type() const
{
    return this->type;
}

const std::string &ColumnInt::get_name() const
{
    return this->name;
}

size_t ColumnInt::size() const
{
    return this->data->size();
}

void ColumnInt::erase(size_t pos)
{
    this->data->erase(this->data->begin() + pos);
}

void ColumnInt::clear()
{
    this->data->clear();
}

void ColumnInt::add(int data)
{
    this->data->push_back(data);
}

int &ColumnInt::get(size_t pos)
{
    if (pos >= this->size()) {
        throw std::logic_error("In ToiletDB, In Column, pos > size of vector");
    }

    return (*(this->data))[pos];
}

std::vector<int> &ColumnInt::get_data()
{
    return *(this->data);
}

ColumnUint::ColumnUint(const std::string name, int type)
{
    TDB_DEBUGS(name, "ColumnB_Int name");
    TDB_DEBUGS(type, "ColumnB_Int type");

    this->name = name;
    this->type = type;
    this->data = new std::vector<size_t>;
}

ColumnUint::~ColumnUint()
{
    delete this->data;
}

const int &ColumnUint::get_type() const
{
    return this->type;
}

const std::string &ColumnUint::get_name() const
{
    return this->name;
}

size_t ColumnUint::size() const
{
    return this->data->size();
}

void ColumnUint::erase(size_t pos)
{
    this->data->erase(this->data->begin() + pos);
}

void ColumnUint::clear()
{
    this->data->clear();
}

void ColumnUint::add(size_t data)
{
    this->data->push_back(data);
}

size_t &ColumnUint::get(size_t pos)
{
    if (pos >= this->size()) {
        throw std::logic_error("In ToiletDB, In Column, pos > size of vector");
    }

    return (*(this->data))[pos];
}

std::vector<size_t> &ColumnUint::get_data()
{
    return *(this->data);
}

ColumnStr::ColumnStr(std::string name, int type)
{
    TDB_DEBUGS(name, "ColumnStr name");
    TDB_DEBUGS(type, "ColumnStr type");

    this->name = name;
    this->type = type;
    this->data = new std::vector<std::string>;
}

ColumnStr::~ColumnStr()
{
    delete this->data;
}

const int &ColumnStr::get_type() const
{
    return this->type;
}

const std::string &ColumnStr::get_name() const
{
    return this->name;
}

size_t ColumnStr::size() const
{
    return this->data->size();
}

void ColumnStr::erase(size_t pos)
{
    this->data->erase(this->data->begin() + pos);
}

void ColumnStr::clear()
{
    this->data->clear();
}

void ColumnStr::add(std::string data)
{
    this->data->push_back(data);
}

std::string &ColumnStr::get(size_t pos)
{
    if (pos >= this->size()) {
        throw std::logic_error("In ToiletDB, In Column, pos > size of vector");
    }

    return (*(this->data))[pos];
}

std::vector<std::string> &ColumnStr::get_data()
{
    return *(this->data);
}

} // namespace toiletdb
