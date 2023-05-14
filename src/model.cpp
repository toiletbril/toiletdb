#pragma once

#include <fstream>
#include <iostream>
#include <vector>

#ifdef DEBUG
#include "debug.cpp"
#endif

#include "parser.cpp"
#include "student.cpp"

class Model
{
private:
    std::vector<Student> students;
    FileParser *parser;

public:
    Model(const char *const &filename)
    {
        this->parser = new FileParser(filename);
        this->parser->exists_or_create();
        this->students = this->parser->read_file();
    }

    ~Model() { delete this->parser; }

    std::vector<Student> &get_all_students() { return this->students; }

    void reread_file() { this->students = this->parser->read_file(); }

    void save_all() { this->parser->write_file(this->students); }

    inline void add(Student &s)
    {
#ifdef DEBUG
        debug_puts(s, "Model.add");
#endif
        this->students.push_back(s);
    }

    void remove_at(unsigned int n)
    {
        this->students.erase(students.begin() + n);
    }

    void clear() { this->students.clear(); }

    size_t size() { return this->students.size(); }
};
