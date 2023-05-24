#pragma once

#include <string>
#include <vector>

#define PARSER_VERSION 1
#define MAGIC "tdb"

#define PARSER_TYPE_MASK 0b00000111

enum FileParserFlags
{
    FINT   = 1 << 0, // int
    FB_INT = 1 << 1, // b_int
    FSTR   = 1 << 2, // str
    FID    = 1 << 3, // id
    FCONST = 1 << 4, // const
};

struct Columns
{
    size_t size;
    // 8 bits flag
    std::vector<int> types;
};

// TODO: what the fuck is this ??
class ParserColumn
{
public:
    virtual int get_type() const         = 0;
    virtual std::string get_name() const = 0;
    virtual size_t size() const          = 0;
    virtual void *get_data()             = 0;
    virtual void erase(size_t pos)       = 0;
    virtual void add(void *data)         = 0;
};

class ParserColumnInt : public ParserColumn
{
    std::vector<int> *data;
    std::string name;

public:
    ParserColumnInt(std::string name)
    {
        this->name = name;
        this->data = new std::vector<int>;
    }

    ~ParserColumnInt()
    {
        delete this->data;
    }

    int get_type() const
    {
        return FINT;
    }

    std::string get_name() const
    {
        return this->name;
    }

    size_t size() const
    {
        return this->data->size();
    }

    void erase(size_t pos)
    {
        this->data->erase(this->data->begin() + pos);
    }

    void add(void *data)
    {
        int number = *static_cast<int *>(data);
        this->data->push_back(number);
    }

    void *get_data()
    {
        return static_cast<void *>(this->data);
    }
};

class ParserColumnB_Int : public ParserColumn
{
    std::vector<unsigned long long> *data;
    std::string name;

public:
    ParserColumnB_Int(const std::string name)
    {
        this->name = name;
        this->data = new std::vector<unsigned long long>;
    }

    ~ParserColumnB_Int()
    {
        delete this->data;
    }

    int get_type() const
    {
        return FB_INT;
    }

    std::string get_name() const
    {
        return this->name;
    }

    size_t size() const
    {
        return this->data->size();
    }

    void erase(size_t pos)
    {
        this->data->erase(this->data->begin() + pos);
    }

    void add(void *data)
    {
        int number = *static_cast<unsigned long long *>(data);
        this->data->push_back(number);
    }

    void *get_data()
    {
        return static_cast<void *>(this->data);
    }
};

class ParserColumnStr : public ParserColumn
{
    std::vector<std::string> *data;
    std::string name;

public:
    ParserColumnStr(std::string name)
    {
        this->name = name;
        this->data = new std::vector<std::string>;
    }

    ~ParserColumnStr()
    {
        delete this->data;
    }

    int get_type() const
    {
        return FSTR;
    }

    std::string get_name() const
    {
        return this->name;
    }

    size_t size() const
    {
        return this->data->size();
    }

    void erase(size_t pos)
    {
        this->data->erase(this->data->begin() + pos);
    }

    void add(void *data)
    {
        std::string string = *static_cast<std::string *>(data);
        this->data->push_back(string);
    }

    void *get_data()
    {
        return static_cast<void *>(this->data);
    }
};
