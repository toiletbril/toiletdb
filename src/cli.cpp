#pragma once

#include <cctype>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#ifdef DEBUG
    #include "debug.cpp"
#endif

#include "common.cpp"
#include "model.cpp"

#define CLI_IDW 12
#define CLI_NAMEW 24
#define CLI_SURNAMEW 24
#define CLI_GROUPW 16
#define CLI_RECORDW 16

#define CLI_WRAPSPACE 4

// TODO: Table wrapping

enum CLI_COMMAND_KIND
{
    UNKNOWN = 0,
    HELP,
    EXIT,
    EXIT_NO_SAVE,
    QUERY,
    ID,
    LIST,
    DBSIZE,
    ADD,
    REMOVE,
    EDIT,
    GRADE,
    CLEAR,
    COMMIT,
    REVERT,
};

// Is this legal ???
void cli_tolower(std::string *str)
{
    for (char &c : *str) {
        c = std::tolower(c);
    }
}

// Prints a prompt which asks to input Y or N.
static bool cli_y_or_n()
{
    while (true) {
        std::cout << "\tY or N >> ";
        std::fflush(stdout);

        char answer = std::tolower(std::fgetc(stdin));

        // Discard the rest of the line.
        while (fgetc(stdin) != '\n') {
        }

        if (answer == 'y') {
            return true;
        } else if (answer == 'n') {
            return false;
        } else {
            std::cout << "ERROR: Invalid character." << std::endl;
        }
    }

    return false;
}

// Splits strings by spaces, treats "quoted sentences" as a single argument.
static std::vector<std::string> cli_splitargs(const std::string &s)
{
    size_t len = s.length();

    std::vector<std::string> result;
    std::string temp;

    bool in_quotes = false;

    for (size_t i = 0; i < len; ++i) {
        char c = s.at(i);

        if (in_quotes) {
            if (c == '"') {
                in_quotes = false;
                continue;
            }
            temp += c;
        } else if (c == '"') {
            in_quotes = true;
        } else if (c == ' ') {
            if (!temp.empty())
                result.push_back(temp);
            temp.clear();
        } else {
            temp += c;
        }
    }
    if (!temp.empty())
        result.push_back(temp);

#ifdef DEBUG
    debug_putv(result, "cli_splitstring");
#endif

    return result;
}

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

static void cli_put_table_header()
{
    std::cout << std::left << std::setw(CLI_IDW) << "ID" << std::setw(CLI_NAMEW)
              << "Name" << std::setw(CLI_SURNAMEW) << "Surname"
              << std::setw(CLI_GROUPW) << "Group" << std::setw(CLI_RECORDW)
              << "Record"
              << "\n";
}

static void cli_put_student(const Student &student)
{
    std::stringstream second_line;

    bool should_wrap = false;

    size_t sub_namew    = CLI_NAMEW - CLI_WRAPSPACE;
    size_t sub_surnamew = CLI_SURNAMEW - CLI_WRAPSPACE;

    if (student.name.length() > sub_namew ||
        student.surname.length() > sub_surnamew)
    {
        should_wrap = true;

        second_line << std::left << std::setw(CLI_IDW) << ' ';

        second_line << std::setw(CLI_NAMEW)
                    << (student.name.length() > sub_namew
                            ? student.name.substr(sub_namew)
                            : " ");

        second_line << std::setw(CLI_SURNAMEW)
                    << (student.surname.length() > sub_surnamew
                            ? student.surname.substr(sub_surnamew)
                            : " ");
    }

    if (!should_wrap) {
        std::cout << std::left << std::setw(CLI_IDW) << student.get_id()
                  << std::setw(CLI_NAMEW) << student.name
                  << std::setw(CLI_SURNAMEW) << student.surname
                  << std::setw(CLI_GROUPW) << student.group
                  << std::setw(CLI_RECORDW) << student.record_book << "\n";
    } else {
        std::cout << std::left << std::setw(CLI_IDW) << student.get_id()
                  << std::setw(CLI_NAMEW) << student.name.substr(0, sub_namew)
                  << std::setw(CLI_SURNAMEW)
                  << student.surname.substr(0, sub_surnamew)
                  << std::setw(CLI_GROUPW) << student.group
                  << std::setw(CLI_RECORDW) << student.record_book << "\n";
        std::cout << second_line.str() << "\n";
    }
}

// Prints entire vector of students as a table.
static void cli_put_student_vector(const std::vector<Student> &students)
{
    cli_put_table_header();

    for (const Student &student : students) {
        cli_put_student(student);
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
    if (s == "id")
        return ID;
    if (s == "list" || s == "ls")
        return LIST;
    if (s == "size")
        return DBSIZE;
    if (s == "add")
        return ADD;
    if (s == "remove" || s == "rm")
        return REMOVE;
    if (s == "grade" || s == "grades")
        return GRADE;
    if (s == "edit" || s == "e")
        return EDIT;
    if (s == "clear")
        return CLEAR;
    if (s == "commit" || s == "save")
        return COMMIT;
    if (s == "revert" || s == "reverse")
        return REVERT;

    return UNKNOWN;
}

// Executes commands based on vector of arguments.
static void cli_exec(Model &model, std::vector<std::string> &args)
{
    CLI_COMMAND_KIND c = cli_getcommand(args[0]);

    switch (c) {
        case UNKNOWN: {
            std::cout << "Unknown command '" << args[0]
                      << "'.\nTry 'help' to see available commands."
                      << std::endl;
        } break;

        case HELP: {
            std::cout
                << "Available commands:\n"
                   "\thelp  \t\tSee this message.\n"
                   "\texit  \t\tQuit and save. Add ! to the end to skip "
                   "saving.\n"
                   "\tsearch\t\tSearch the database.\n"
                   "\tid    \t\tSearch by ID.\n"
                   "\tlist  \t\tList all students.\n"
                   "\tsize  \t\tSee total amount of students in database.\n"
                   "\tadd   \t\tAdd a student to database.\n"
                   "\tremove\t\tRemove a student from database.\n"
                   "\tedit  \t\tEdit student's details.\n"
                   "\tgrades\t\tSee student's grades.\n"
                   "\tclear \t\tClear the database.\n"
                   "\tcommit\t\tSave changes to the file.\n"
                   "\trevert\t\tRevert uncommited changes."
                << std::endl;
        } break;

        case EXIT: {
            std::cout << "Saving..." << std::endl;
            model.save_all();
            std::cout << "Exiting...\n" << std::endl;
            std::exit(0);
        } break;

        case EXIT_NO_SAVE: {
            std::cout << "Exiting...\n" << std::endl;
            std::exit(0);
        } break;

        case LIST: {
            size_t len = model.size();

            if (len > 1000) {
                std::cout << "Database has over 1 000 entries (" << len
                          << ").\n"
                             "This can take a long time.\n"
                             "Do you really want to list them all?\n";

                if (!cli_y_or_n()) {
                    return;
                }
            }

            const std::vector<Student> &students = model.get_all_students();

            if (students.empty()) {
                std::cout << "There are no students in database." << std::endl;
                return;
            }

            cli_put_student_vector(students);
            std::fflush(stdout);
        } break;

        case DBSIZE: {
            size_t len = model.size();

            std::cout << "There are " << len << " students in database."
                      << std::endl;
        } break;

        case ADD: {
            char errc = cli_forbiddenchar(args);

            if (errc) {
                std::cout << "ERROR: Forbidden character '" << errc << "'."
                          << std::endl;
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
                       "Humans\" 69420"
                    << std::endl;
                return;
            }

            Student student(model.get_next_id(), args[1].c_str(),
                            args[2].c_str(), args[3].c_str(), args[4].c_str());

            model.add(student);

            std::cout << "Student added successfully.\n";
        } break;

        case REMOVE: {
            if (args.size() != 2) {
                std::cout << "ERROR: Invalid number of arguments.\n"
                             "Usage: remove <student ID>\nYou can get "
                             "student ID by using 'list' or 'query'."
                          << std::endl;
                return;
            }

            size_t n = cm_parsell(args[1]);

            if (n == COMMON_INVALID_NUMBERLL) {
                std::cout << "ERROR: Invalid number." << std::endl;
                return;
            }

            // TODO: Replace this with remove(id)
            bool success = model.remove_id(n);

            if (success)
                std::cout << "Student removed successfully." << std::endl;
            else
                std::cout << "Could not find student with such ID."
                          << std::endl;
        } break;

        case GRADE: {
            throw std::logic_error("TODO");
        } break;

        case EDIT: {
            throw std::logic_error("TODO");
        } break;

        case CLEAR: {
            std::cout << "Do you really want to do that?\n";
            if (!cli_y_or_n())
                return;

            model.clear();
            std::cout << "Database has been erased." << std::endl;
        } break;

        case COMMIT: {
            std::cout << "Saving..." << std::endl;
            model.save_all();
        } break;

        case REVERT: {
            std::cout << "Reverting changes..." << std::endl;
            model.reread_file();
        } break;

        case QUERY: {
            // Search by name prefix
            if (args.size() < 2) {
                std::cout << "ERROR: Invalid number of arguments.\n"
                             "Usage: query <student name>\n";

                return;
            }

            std::cout << "Searching...\n\n";

            std::string query = args[1];
            size_t len        = args.size();

            // Concat args to a single string and use it as a query.
            for (size_t i = 2; i < len; ++i) {
                query += ' ' + args[i];
            }

            cli_tolower(&query);

#ifdef DEBUG
            debug_puts(query, "query");
#endif
            const std::vector<Student> &students = model.get_all_students();

            // TODO:
            // Using a vector for printing is a waste of memory.
            // But this will not be used by anyone, anyways.
            std::vector<Student> result;

            for (const Student &s : students) {
                std::string full_name = s.name + " " + s.surname;
                cli_tolower(&full_name);

                if (full_name.rfind(query, 0) == 0) {
                    result.push_back(s);
                }
            }

            if (result.size() == 0) {
                std::cout << "No students matched your query.\n";
                return;
            }

            cli_put_student_vector(result);
        } break;

        case ID: {
            if (args.size() != 2) {
                std::cout << "ERROR: Invalid number of arguments.\n"
                             "Usage: id <student ID>\nYou can get "
                             "student ID by using 'list' or 'query'.\n";
                return;
            }

            size_t n = cm_parsell(args[1]);

            if (n == COMMON_INVALID_NUMBERLL) {
                std::cout << "ERROR: Invalid number.\n";
                return;
            }

            size_t result = model.search(n);

            if (result == MODEL_NOT_FOUND) {
                std::cout << "No students matched your query.\n";
                return;
            }

            cli_put_table_header();
            cli_put_student(model.get(result));
            std::fflush(stdout);
        } break;
    }
}

void cli_loop(const char *filename)
{
    int err = std::setvbuf(stdout, NULL, _IOFBF, 2048);

    if (err) {
        std::cout << "Could not enable bufferin for output.\n Please buy more "
                     "memory :c"
                  << std::endl;
        exit(1);
    }

    Model *model;

    try {
        std::cout << "Opening '" << filename << "'..." << std::endl;
        model = new Model(filename);
    }
    catch (std::ios::failure &e) {
        // iostream error's .what() method returns weird string at the end
        std::cout << filename << ": Could not open file: " << strerror(errno)
                  << std::endl;
        std::exit(1);
    }
    catch (std::range_error &e) {
        std::cout << filename << ": Parsing error: " << e.what() << std::endl;
        exit(1);
    }

    std::cout << "\nWelcome to toiletdb.\n"
                 "Loaded "
              << model->size()
              << " students.\n"
                 "Try 'help' to see available commands."
              << std::endl;

    while (true) {
        std::cout << "\n" << filename << "# ";

        static std::string line;
        std::getline(std::cin, line);

        std::vector<std::string> args = cli_splitargs(line);

        try {
            cli_exec(*model, args);
        }
        catch (std::logic_error &e) {
            std::cout << "Logic error: " << e.what() << std::endl;
        }
        catch (std::ios::failure &e) {
            std::cout << "IO error: " << strerror(errno) << std::endl;
            std::exit(1);
        }
        catch (std::runtime_error &e) {
            std::cout << "Runtime error: " << e.what() << std::endl;
            std::exit(1);
        }
    }
}
