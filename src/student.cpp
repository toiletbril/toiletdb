#pragma once

#include <ostream>
#include <string>

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
            const char *group, const char *record_book) :
        id(id),
        name(name), surname(surname), group(group), record_book(record_book)
    {}

    size_t get_id() const
    {
        return this->id;
    }

    friend std::ostream &operator<<(std::ostream &os, const Student &s)
    {
        os << "Student { " << std::to_string(s.get_id()) << ", " << s.name
           << ", " << s.surname << ", " << s.group << ", " << s.record_book
           << " }";
        return os;
    }
};
