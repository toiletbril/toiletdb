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

    void remove_at(size_t n) { this->students.erase(students.begin() + n); }

    bool remove_id(size_t id)
    {
        std::vector<Student> &students = this->students;
        size_t i = 0;

        for (const Student &s : students) {
            if (s.get_id() == id) {
#ifdef DEBUG
                debug_puts(s, "Model.remove_id");
#endif
                students.erase(students.begin() + i);
                return true;
            }
            ++i;
        }

        return false;
    }

    void clear() { this->students.clear(); }

    size_t size() { return this->students.size(); }

    size_t get_next_id() { return this->size() + 1; }
};
