#pragma once

#include <iostream>
#include <string>

struct Student
{
    std::string name;
    std::string surname;
    std::string group;
    std::string record_book;

    Student(const char *name, const char *surname, const char *group,
            const char *record_book)
    {
        this->name = name;
        this->surname = surname;
        this->group = group;
        this->record_book = record_book;
    }

    Student() {}

    // Allows to use << operator to print out this class.
    friend std::ostream &operator<<(std::ostream &os, const Student &s)
    {
        os << "Student { " << s.name << ", " << s.surname << ", " << s.group
           << ", " << s.record_book << " }";
        return os;
    }
};
