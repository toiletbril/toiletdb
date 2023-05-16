#pragma once

#include <algorithm>
#include <fstream>
#include <iostream>
#include <vector>

#ifdef DEBUG
    #include "debug.cpp"
#endif

#include "parser.cpp"
#include "student.cpp"

#define MODEL_NOT_FOUND (size_t)(-1)

class Model
{
private:
    std::vector<Student> students;
    FileParser *parser;

    void erase(const size_t n)
    {
        this->students.erase(students.begin() + n);
    }

public:
    // This will create a file 'filename' and use it to store commits.
    // If file already exists, elements will be sorted by ID to suit binary
    // search.
    // Using previously unsorted file will result in it being reordered.
    Model(const char *const &filename)
    {
        this->parser = new FileParser(filename);
        this->parser->exists_or_create();
        this->students = this->parser->read_file();

        std::sort(this->students.begin(), this->students.end(),
                  [](const Student &a, const Student &b) {
                      return a.get_id() < b.get_id();
                  });
    }

    ~Model()
    {
        delete this->parser;
    }

    const std::vector<Student> &get_all_students() const
    {
        return this->students;
    }

    Student &get(const size_t n)
    {
        if (n >= this->size()) {
            throw std::range_error("n is too large");
        }

        return this->students[n];
    }

    void reread_file()
    {
        this->students = this->parser->read_file();
    }

    void save_all() const
    {
        this->parser->write_file(this->students);
    }

    // Return index of element in Model.
    // If element is not found, returns MODEL_NOT_FOUND.
    size_t search(const size_t id) const
    {
        size_t L = 0;
        size_t R = this->size();
        size_t m = -1;

        while (L <= R) {
            m = std::floor((L + R) / 2);

            if (this->students[m].get_id() < id) {
                L = m + 1;
            } else if (this->students[m].get_id() > id) {
                R = m - 1;
            } else {
                return m;
            }
        }

        return MODEL_NOT_FOUND;
    }

    inline void add(Student &s)
    {
#ifdef DEBUG
        debug_puts(s, "Model.add");
#endif
        if (this->search(s.get_id()) != MODEL_NOT_FOUND) {
            throw std::logic_error("An entry exists with same ID");
        }

        this->students.push_back(s);
    }

    bool remove_id(const size_t id)
    {
        size_t result = this->search(id);

        if (result != MODEL_NOT_FOUND) {
            this->erase(result);
            return true;
        }

        return false;
    }

    inline void clear()
    {
        this->students.clear();
    }

    size_t size() const
    {
        return this->students.size();
    }

    size_t get_next_id() const
    {
        return this->size();
    }
};
