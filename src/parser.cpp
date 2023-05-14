#pragma once

#include <cstring>
#include <fstream>
#include <iostream>
#include <vector>

#ifdef DEBUG
#include "debug.cpp"
#endif

#include "student.cpp"

class Filer
{
private:
    const char *const &filename;

    std::fstream open(std::ios_base::openmode mode)
    {
        std::fstream file;

        file.open(this->filename, mode);

#ifdef DEBUG
        debug_puts(filename, "open");
#endif

        if (!file.is_open()) {
            throw std::ios::failure("Could not open file");
        }

        return file;
    }

    // Read file from disk into memory.
    std::vector<Student> deserealize(std::fstream &file)
    {
        std::vector<Student> students;
        std::string temp;

        size_t line = 1;
        size_t pos = 1;

        int c = file.get();

        while (c != EOF) {
            if (c != '|') {
                std::string failstring = "Database file format is not "
                                         "correct: invalid delimiter at line " +
                                         std::to_string(line) + ":" +
                                         std::to_string(pos);

                throw std::range_error(failstring);
            } else {
                c = file.get();
            }

            Student tempst;
            int field = 0;

            while (c != '\n' && c != '\r') {

                if (c == '|') {
                    std::string &temps = temp;

                    switch (field) {
                        case 0:
                            tempst.name = temps.c_str();
                            break;
                        case 1:
                            tempst.surname = temps.c_str();
                            break;
                        case 2:
                            tempst.group = temps.c_str();
                            break;
                        case 3:
                            tempst.record_book = temps.c_str();
                            break;

                        default: {
                            std::string failstring =
                                "Database file format is not "
                                "correct: extra field at line " +
                                std::to_string(line) + ":" +
                                std::to_string(pos + 1);

                            throw std::range_error(failstring);
                        }
                    }

                    temp.clear();
                    ++field;
                } else {
                    temp += c;
                }

                c = file.get();
                ++pos;
            }

            if (field != 4) {
                std::string failstring = "Database file format is not "
                                         "correct: invalid fields (" +
                                         std::to_string(field) +
                                         "/4) at line " + std::to_string(line) +
                                         ":" + std::to_string(pos);

                throw std::range_error(failstring);
            }

#ifdef DEBUG
            debug_puts(tempst, "deserealize");
#endif

            students.push_back(tempst);

            c = file.get();
            ++line;
            pos = 1;
        }
        return students;
    }

    // Save vector of students from memory into file.
    void serialize(std::vector<Student> students, std::fstream &file)
    {
        for (Student student : students) {
#ifdef DEBUG
            debug_puts(student, "serealize");
#endif

            file << '|' << student.name << '|' << student.surname << '|'
                 << student.group << '|' << student.record_book << '|' << "\n";
        }

        file.flush();
    }

public:
    Filer(const char *const &filename) : filename(filename) {}

    ~Filer() { delete this->filename; }

    // Return true if file exists, false if it doesn't.
    bool exists()
    {
        std::fstream file;

        file.open(this->filename);

#ifdef DEBUG
        debug_puts(file.good(), "exists");
#endif

        return file.good();
    }

    // Return true if file exists, false if it doesn't.
    // Creates file if it didn't exist.
    bool exists_or_create()
    {
        if (!this->exists()) {
            std::fstream file;
            file.open(this->filename, std::ios::out);

            if (!file.good()) {
                throw std::ios::failure("Could not create file");
            }

            return false;
        }

        return true;
    }

    std::vector<Student> read_file()
    {
        std::fstream file;

        file = this->open(std::ios::in | std::ios::out | std::ios::binary);

        std::vector<Student> students = this->deserealize(file);
        file.close();

        return students;
    }

    void write_file(std::vector<Student> &students)
    {
        if (!this->exists()) {
            throw std::runtime_error("File does not exist");
        }

        std::fstream file =
            this->open(std::ios::out | std::ios::trunc | std::ios::binary);

        this->serialize(students, file);
        file.close();
    }
};
