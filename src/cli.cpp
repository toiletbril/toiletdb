#pragma once

#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <vector>

#ifdef DEBUG
#include "debug.cpp"
#endif

#include "model.cpp"

enum CLI_COMMAND_KIND
{
    UNKNOWN = 0,
    HELP,
    EXIT,
    EXIT_NO_SAVE,
    QUERY,
    LIST,
    ADD,
    REMOVE,
    GRADE,
    CLEAR,
    COMMIT,
    REVERT,
};

// Prints a prompt which asks to input Y or N.
static bool cli_y_or_n()
{
    while (true) {
        std::cout << "\tY or N > ";
        char answer = std::tolower(std::fgetc(stdin));

        // Discard the rest of the line.
        while (fgetc(stdin) != '\n') {
        }

        if (answer == 'y') {
            return true;
        } else if (answer == 'n') {
            return false;
        } else {
            std::cout << "ERROR: Invalid character.\n";
        }
    }

    return false;
}

// Splits strings by spaces, treats "quotes sentences" as a single argument.
static std::vector<std::string> cli_splitargs(const std::string *s)
{
    size_t len = s->length();

    std::vector<std::string> result;
    std::string temp;

    bool in_quotes = false;

    for (size_t i = 0; i < len; ++i) {
        char c = s->at(i);

        if (in_quotes) {
            if (c == '"') {
                in_quotes = false;
                continue;
            }
            temp += c;
        } else if (c == '"') {
            in_quotes = true;
        } else if (c == ' ') {
            result.push_back(temp);
            temp.clear();
        } else {
            temp += c;
        }
    }
    result.push_back(temp);

#ifdef DEBUG
    debug_putv(result, "cli_splitstring");
#endif

    return result;
}

// As an example, model uses "|" as a separator.
// Returns first invalid character it finds.
static char cli_forbiddenchar(std::vector<std::string> &args)
{
    std::string forbidden_chars = "|[]";
    for (const std::string &a : args) {
        for (char const c : a) {
            if (forbidden_chars.find(c) != std::string::npos) {
                return c;
            }
        }
    }
    return 0;
}

// Prints entire vector of students.
static void cli_putquery(std::vector<Student> &students)
{
    std::cout << std::left << std::setw(12) << "N" << std::setw(24) << "Name"
              << std::setw(30) << "Surname" << std::setw(12) << "Group"
              << std::setw(12) << "Record"
              << "\n";

    int n = 0;

    for (Student student : students) {
        std::cout << std::left << std::setw(12) << ++n << std::setw(24)
                  << student.name << std::setw(30) << student.surname
                  << std::setw(12) << student.group << std::setw(12)
                  << student.record_book << "\n";
    }
}

// Translates strings to CLI_COMMAND_KIND.
static CLI_COMMAND_KIND cli_getcommand(std::string s)
{
    if (s == "help" || s == "?")
        return HELP;
    if (s == "exit" || s == "quit")
        return EXIT;
    if (s == "exit!" || s == "quit!" || s == "q!")
        return EXIT_NO_SAVE;
    if (s == "query" || s == "search")
        return QUERY;
    if (s == "list" || s == "ls")
        return LIST;
    if (s == "add")
        return ADD;
    if (s == "remove" || s == "rm")
        return REMOVE;
    if (s == "grade")
        return GRADE;
    if (s == "clear")
        return CLEAR;
    if (s == "commit" || s == "save")
        return COMMIT;
    if (s == "revert" || s == "reverse")
        return REVERT;

    return UNKNOWN;
}

// Executes commands based on vector of arguments.
static void cli_exec(Model *model, std::vector<std::string> args)
{
    CLI_COMMAND_KIND c = cli_getcommand(args[0]);

    switch (c) {
        case UNKNOWN: {
            std::cout << "Unknown command '" << args[0]
                      << "'.\nTry 'help' to see available commands.\n";
        } break;

        case HELP: {
            std::cout
                << "Available commands:\n"
                   "\thelp  \tSee this message.\n"
                   "\texit  \tQuit and save. Add ! to the end to skip saving.\n"
                   "\tquery \tQuery database.\n"
                   "\tlist  \tList all students.\n"
                   "\tadd   \tAdd a student to database.\n"
                   "\tremove\tRemove a student from database.\n"
                   "\tgrade \tAdd student's grades.\n"
                   "\tclear \tClear the database.\n"
                   "\tcommit\tSave changes to the file.\n"
                   "\trevert\tRevert uncommited changes.\n";
        } break;

        case EXIT: {
            std::cout << "Saving...\n";
            model->save_all();
            std::cout << "Exiting...\n\n";
            std::exit(0);
        } break;

        case EXIT_NO_SAVE: {
            std::cout << "Exiting...\n\n";
            std::exit(0);
        } break;

        case LIST: {
            size_t len = model->size();

            if (len > 1000) {
                std::cout << "Database has over 1000 entries (" << len
                          << ").\n"
                             "This can take a long time.\n"
                             "Do you really want to list them all?\n";

                if (!cli_y_or_n()) {
                    return;
                }
            }

            std::vector<Student> students = model->get_all_students();

            if (students.empty()) {
                std::cout << "There is no students in database."
                          << "\n";
                return;
            }

            cli_putquery(students);
        } break;

        case ADD: {
            char errc = cli_forbiddenchar(args);

            if (errc) {
                std::cout << "ERROR: Forbidden character '" << errc << "'.\n";
                return;
            }

            size_t len = args.size();

            if (len != 5) {
                std::cout
                    << "ERROR: Invalid number of arguments. "
                       "(5 needed, actual "
                    << len
                    << ")\n"
                       "Usage: add <name> <surname> <group> <record book>\n"
                       "You can put quotes around fields.\n"
                       "EXAMPLE: \"Vasiliy Dos\" \"Super Test\" \"Very "
                       "Humans\" 69420\n";
                return;
            }

            Student student(args[1].c_str(), args[2].c_str(), args[3].c_str(),
                            args[4].c_str());

            model->add(student);

            std::cout << "Student added successfully.\n";
        } break;

        case REMOVE: {
            if (args.size() != 2) {
                std::cout << "ERROR: Invalid number of arguments.\n"
                             "Usage: remove <student index>\nYou can get "
                             "student index by using 'list'.\n";
                return;
            }

            size_t n = std::atoll(args[1].c_str());

            if (n > model->size()) {
                std::cout << "ERROR: index is too big.\n";
                return;
            }

            if (n <= 0) {
                std::cout << "ERROR: invalid number.\n";
                return;
            }

            model->remove_at(n - 1);

            std::cout << "Student removed successfully.\n";
        } break;

        case GRADE: {
            throw std::logic_error("Not implemented");
        } break;

        case CLEAR: {
            std::cout << "Do you really want to do that?\n";
            if (!cli_y_or_n())
                return;

            model->clear();
            std::cout << "Database has been erased.\n";
        } break;

        case COMMIT: {
            std::cout << "Saving...\n";
            model->save_all();
        } break;

        case REVERT: {
            std::cout << "Reverting changes...\n";
            model->reread_file();
        } break;

        case QUERY: {
            throw std::logic_error("Not implemented");
        } break;
    }
}

void cli_loop(const char *filename)
{
    Model *model;

    try {
        std::cout << "Opening '" << filename << "'...\n";
        model = new Model(filename);
    } catch (std::ios::failure &e) {
        std::cout << filename << ": Could not open file: " << strerror(errno)
                  << "\n";
        std::exit(1);
    }

    std::cout << "Welcome to toiletdb.\n"
                 "Loaded "
              << model->size()
              << " students.\n"
                 "Try 'help' to see available commands."
              << std::endl;

    while (true) {
        std::cout << "\n" << filename << " # ";

        static char line[128];
        std::cin.getline(line, 128);

        std::string line_string = line;

        std::vector<std::string> args = cli_splitargs(&line_string);

        try {
            cli_exec(model, args);
        } catch (std::logic_error &e) {
            std::cout << "Logic error: " << e.what() << "\n";
        } catch (std::ios::failure &e) {
            // iostream .what() method returns weird string at the end, but I
            // want errors to look clear.
            std::cout << "IO error: " << strerror(errno) << "\n";
            std::exit(1);
        } catch (std::runtime_error &e) {
            std::cout << "Runtime error: " << e.what() << "\n";
            std::exit(1);
        }
    }
}
