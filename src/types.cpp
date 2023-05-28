#define TOILETDB_MAGIC "tdb"

#define TDB_TMASK 0b00000111
#define TDB_MMASK 0b00111000

#include "types.hpp"

namespace toiletdb {

enum ToiletType;

struct TableInfo;

// TODO: what the fuck is this ??

class Column;

ColumnInt::ColumnInt(std::string name, int type)
{
    TOILET_DEBUGS(name, "ColumnInt name");
    TOILET_DEBUGS(type, "ColumnInt type");

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

void ColumnInt::add(void *data)
{
    int number = *static_cast<int *>(data);
    this->data->push_back(number);
}

void *ColumnInt::get(size_t pos)
{
    if (pos >= this->size()) {
        throw std::logic_error("In ToiletDB, In Column, pos > size of vector");
    }

    return static_cast<void *>(&(*(this->data))[pos]);
}

void *ColumnInt::get_data()
{
    return static_cast<void *>(this->data);
}

ColumnB_Int::ColumnB_Int(const std::string name, int type)
{
    TOILET_DEBUGS(name, "ColumnB_Int name");
    TOILET_DEBUGS(type, "ColumnB_Int type");

    this->name = name;
    this->type = type;
    this->data = new std::vector<unsigned long long>;
}

ColumnB_Int::~ColumnB_Int()
{
    delete this->data;
}

const int &ColumnB_Int::get_type() const
{
    return this->type;
}

const std::string &ColumnB_Int::get_name() const
{
    return this->name;
}

size_t ColumnB_Int::size() const
{
    return this->data->size();
}

void ColumnB_Int::erase(size_t pos)
{
    this->data->erase(this->data->begin() + pos);
}

void ColumnB_Int::clear()
{
    this->data->clear();
}

void ColumnB_Int::add(void *data)
{
    unsigned long long number = *static_cast<unsigned long long *>(data);
    this->data->push_back(number);
}

void *ColumnB_Int::get(size_t pos)
{
    if (pos >= this->size()) {
        throw std::logic_error("In ToiletDB, In Column, pos > size of vector");
    }

    return static_cast<void *>(&(*(this->data))[pos]);
}

void *ColumnB_Int::get_data()
{
    return static_cast<void *>(this->data);
}

ColumnStr::ColumnStr(std::string name, int type)
{
    TOILET_DEBUGS(name, "ColumnStr name");
    TOILET_DEBUGS(type, "ColumnStr type");

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

void ColumnStr::add(void *data)
{
    std::string string = *static_cast<std::string *>(data);
    this->data->push_back(string);
}

void *ColumnStr::get(size_t pos)
{
    if (pos >= this->size()) {
        throw std::logic_error("In ToiletDB, In Column, pos > size of vector");
    }

    return static_cast<void *>(&(*(this->data))[pos]);
}

void *ColumnStr::get_data()
{
    return static_cast<void *>(this->data);
}

} // namespace toiletdb
