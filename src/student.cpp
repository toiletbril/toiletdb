#pragma once

#include <iostream>
#include <string>

#define TEMP_ID -1

class Student
{
private:
    size_t id;

public:
    std::string name;
    std::string surname;
    std::string group;
    std::string record_book;

    Student(const size_t id, const char *name, const char *surname,
            const char *group, const char *record_book)
        : id(id)
    {
        this->name = name;
        this->surname = surname;
        this->group = group;
        this->record_book = record_book;
    }

    Student(const size_t id) : id(id) {}

    size_t get_id() const { return this->id; }

    // Allows to use << operator to print out this class.
    friend std::ostream &operator<<(std::ostream &os, const Student &s)
    {
        os << "Student { " << std::to_string(s.get_id()) << ", " << s.name
           << ", " << s.surname << ", " << s.group << ", " << s.record_book
           << " }";
        return os;
    }
};
