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

#define CLI_SUB_NAMEW (CLI_NAMEW - CLI_WRAPSPACE)
#define CLI_SUB_SURNAMEW (CLI_SURNAMEW - CLI_WRAPSPACE)
#define CLI_SUB_GROUPW (CLI_GROUPW - CLI_WRAPSPACE)

enum CLI_COMMAND_KIND
{
    UNKNOWN = 0,
    HELP,
    EXIT,
    EXIT_NO_SAVE,
    QUERY,
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

// Extracts filename from file path.
static std::string cli_extract_filename(const std::string &path)
{
#ifdef _WIN32
    char delimiter[] = "\\/";
#else
    char delimiter = '/';
#endif
    return path.substr(path.find_last_of(delimiter) + 1);
}

// Concatenates args from pos to the end into a single string.
static std::string cli_concat_args(std::vector<std::string> &args, size_t pos)
{
    size_t len         = args.size();
    std::string result = args[pos];

    for (size_t i = pos + 1; i < len; ++i)
    {
        result += ' ' + args[i];
    }

    return result;
}

// Splits strings by spaces, treats "quoted sentences" as a single argument.
static std::vector<std::string> cli_split_args(const std::string &s)
{
    size_t len = s.length();

    std::vector<std::string> result;
    std::string temp;

    bool in_quotes = false;

    for (size_t i = 0; i < len; ++i)
    {
        char c = s.at(i);

        if (in_quotes)
        {
            if (c == '"')
            {
                in_quotes = false;
                continue;
            }
            temp += c;
        }
        else if (c == '"')
        {
            in_quotes = true;
        }
        else if (c == ' ')
        {
            if (!temp.empty())
            {
                result.push_back(temp);
            }
            temp.clear();
        }
        else
        {
            temp += c;
        }
    }
    if (!temp.empty())
    {
        result.push_back(temp);
    }

#ifdef DEBUG
    debug_putv(result, "cli_splitstring");
#endif

    return result;
}

// Prints a prompt which asks to input Y or N.
static bool cli_y_or_n()
{
    while (true)
    {
        std::cout << "\tY or N >> ";
        std::fflush(stdout);

        char answer = std::tolower(std::fgetc(stdin));

        // Discard the rest of the line.
        while (fgetc(stdin) != '\n')
        {}

        if (answer == 'y')
        {
            return true;
        }
        else if (answer == 'n')
        {
            return false;
        }
        else
        {
            std::cout << "ERROR: Invalid character." << std::endl;
        }
    }

    return false;
}

// Returns first invalid character it finds.
static char cli_forbiddenchar(std::vector<std::string> &args)
{
    std::string forbidden_chars = "|[]";
    for (const std::string &a : args)
    {
        for (char const &c : a)
        {
            if (forbidden_chars.find(c) != std::string::npos)
            {
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

// Prints out a student as column.
// Wraps words by breaking them to the next line if they exceed column width.
static void cli_put_student(const Student &student)
{
    std::stringstream wrap_buf;

    bool should_wrap = false;

    size_t line = 1;

    int namelen    = student.name.length();
    int surnamelen = student.surname.length();
    int grouplen   = student.group.length();

    while (namelen > CLI_SUB_NAMEW || surnamelen > CLI_SUB_SURNAMEW ||
           grouplen > CLI_SUB_GROUPW)
    {
        should_wrap = true;

        if (line > 1)
        {
            wrap_buf << "\n";
        }

        wrap_buf << std::left << std::setw(CLI_IDW) << ' ';

        wrap_buf << std::setw(CLI_NAMEW)
                 << (namelen > CLI_SUB_NAMEW
                         ? student.name.substr(CLI_SUB_NAMEW * line,
                                               CLI_SUB_NAMEW)
                         : " ");

        wrap_buf << std::setw(CLI_SURNAMEW)
                 << (surnamelen > CLI_SUB_SURNAMEW
                         ? student.surname.substr(CLI_SUB_SURNAMEW * line,
                                                  CLI_SUB_SURNAMEW)
                         : " ");

        wrap_buf << std::setw(CLI_GROUPW)
                 << (grouplen > CLI_SUB_GROUPW
                         ? student.group.substr(CLI_SUB_GROUPW * line,
                                                CLI_SUB_GROUPW)
                         : " ");

        namelen    = namelen - CLI_SUB_NAMEW;
        surnamelen = surnamelen - CLI_SUB_SURNAMEW;
        grouplen   = grouplen - CLI_SUB_GROUPW;

        ++line;
    }

    if (!should_wrap)
    {
        std::cout << std::left << std::setw(CLI_IDW) << student.get_id()
                  << std::setw(CLI_NAMEW) << student.name
                  << std::setw(CLI_SURNAMEW) << student.surname
                  << std::setw(CLI_GROUPW) << student.group
                  << std::setw(CLI_RECORDW) << student.record_book << "\n";
    }
    else
    {
        std::cout << std::left << std::setw(CLI_IDW) << student.get_id()
                  << std::setw(CLI_NAMEW)
                  << student.name.substr(0, CLI_SUB_NAMEW)
                  << std::setw(CLI_SURNAMEW)
                  << student.surname.substr(0, CLI_SUB_SURNAMEW)
                  << std::setw(CLI_GROUPW)
                  << student.group.substr(0, CLI_SUB_GROUPW)
                  << std::setw(CLI_RECORDW) << student.record_book << "\n";
        std::cout << wrap_buf.str() << "\n";
    }
}

// Prints entire vector of students as a table.
static void cli_put_student_vector(const std::vector<Student> &students)
{
    cli_put_table_header();

    for (const Student &student : students)
    {
        cli_put_student(student);
    }
}

// Translates strings to CLI_COMMAND_KIND.
static CLI_COMMAND_KIND cli_getcommand(std::string &s)
{
    if (s == "help" || s == "?")
        return HELP;
    if (s == "exit" || s == "quit" || s == "q")
        return EXIT;
    if (s == "exit!" || s == "quit!" || s == "q!")
        return EXIT_NO_SAVE;
    if (s == "search" || s == "s")
        return QUERY;
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
static void cli_exec(InMemoryModel &model, std::vector<std::string> &args)
{
    CLI_COMMAND_KIND c = cli_getcommand(args[0]);

    switch (c)
    {
        case UNKNOWN: {
            std::cout << "ERROR: Unknown command '" << args[0]
                      << "'.\nTry 'help' to see available commands."
                      << std::endl;
        }
        break;

        case HELP: {
            std::cout
                << "Available commands:\n"
                   "\thelp  \t?\t\tSee this message.\n"
                   "\texit  \tq, quit\t\tSave and quit. "
                   "Append '!' to the end to skip saving.\n"
                   "\tsearch\ts\t\tSearch the database.\n"
                   "\tlist  \tls\t\tList all students.\n"
                   "\tsize  \t\t\tSee total amount of students in database.\n"
                   "\tadd   \t\t\tAdd a student to database.\n"
                   "\tremove\trm\t\tRemove a student from database.\n"
                   "\tedit  \te\t\tEdit student's details.\n"
                   "\tgrades\tgrade\t\tSee student's grades.\n"
                   "\tclear \t\t\tClear the database.\n"
                   "\tcommit\tsave\t\tSave changes to the file.\n"
                   "\trevert\treverse\t\tRevert uncommited changes."
                << std::endl;
        }
        break;

        case EXIT: {
            std::cout << "Saving..." << std::endl;
            model.write_file();
            std::cout << "Exiting...\n" << std::endl;
            std::exit(0);
        }
        break;

        case EXIT_NO_SAVE: {
            std::cout << "Exiting...\n" << std::endl;
            std::exit(0);
        }
        break;

        case LIST: {
            size_t len = model.size();

            if (len > 1000)
            {
                std::cout << "Database has over 1 000 entries (" << len
                          << ").\n"
                             "This can take a long time.\n"
                             "Do you really want to list them all?\n";

                if (!cli_y_or_n())
                {
                    return;
                }
            }

            const std::vector<Student> &students = model.get_all();

            if (students.empty())
            {
                std::cout << "There are no students in database." << std::endl;
                return;
            }

            cli_put_student_vector(students);
            std::fflush(stdout);
        }
        break;

        case QUERY: {
            if (args.size() < 3)
            {
                std::cout << "ERROR: Not enough arguments.\n"
                             "USAGE: search <field> <value>\n"
                             "Available fields: 'id', 'record', 'name'\n";

                return;
            }

            std::cout << "Searching...\n\n";

            std::string query = cli_concat_args(args, 2);

            cm_pstr_tolower(query);

#ifdef DEBUG
            debug_puts(query, "search");
#endif
            std::vector<size_t> result;

            if (cm_str_tolower(args[1]) == "id")
            {
                size_t id = cm_parsell(query);

                if (id == COMMON_INVALID_NUMBERLL)
                {
                    std::cout << "ERROR: ID is not a number.\n";
                    return;
                }

                size_t pos = model.search(id);

                if (pos == MODEL_NOT_FOUND)
                {
                    std::cout << "No students matched your query." << std::endl;
                    return;
                }

                result.push_back(pos);
            }
            else if (cm_str_tolower(args[1]) == "record")
            {
                result = model.search_record(query);
            }
            else if (cm_str_tolower(args[1]) == "name")
            {
                result = model.search(query.c_str());
            }
            else
            {
                std::cout << "Unknown field '" << args[1]
                          << "'.\n"
                             "Available fields: 'id', 'record', 'name'\n";
                return;
            }

            if (result.size() == 0)
            {
                std::cout << "No students matched your query." << std::endl;
                return;
            }

            cli_put_table_header();

            for (size_t &pos : result)
            {
                cli_put_student(model.get(pos));
            }

            std::fflush(stdout);
        }
        break;

        case DBSIZE: {
            size_t len = model.size();

            std::cout << "There are " << len << " students in database."
                      << std::endl;
        }
        break;

        case ADD: {
            char errc = cli_forbiddenchar(args);

            if (errc)
            {
                std::cout << "ERROR: Forbidden character '" << errc << "'."
                          << std::endl;
                return;
            }

            size_t len = args.size();

            if (len != 5)
            {
                std::cout
                    << "ERROR: Invalid number of arguments. "
                    << "(5 needed, actual " << len << ")\n"
                    << "USAGE: add <name> <surname> <group> <record book>\n"
                       "You can put quotes around fields.\n"
                       "EXAMPLE: "
                       "Vasiliy \"Ivanov Petrov\" \"Long Group\" 69420"
                    << std::endl;
                return;
            }

            Student student(model.get_next_id(), args[1].c_str(),
                            args[2].c_str(), args[3].c_str(), args[4].c_str());

            model.add(student);

            std::cout << "Student added successfully.\n";
        }
        break;

        case REMOVE: {
            if (args.size() != 2)
            {
                std::cout << "ERROR: Invalid number of arguments.\n"
                             "USAGE: remove <student ID>\nYou can get "
                             "student ID by using 'list' or 'query'."
                          << std::endl;
                return;
            }

            size_t n = cm_parsell(args[1]);

            if (n == COMMON_INVALID_NUMBERLL)
            {
                std::cout << "ERROR: Invalid ID." << std::endl;
                return;
            }

            bool success = model.erase_id(n);

            if (success)
                std::cout << "Student removed successfully." << std::endl;
            else
                std::cout << "Could not find student with such ID."
                          << std::endl;
        }
        break;

        case EDIT: {
            if (args.size() < 3)
            {
                std::cout
                    << "ERROR: Not enough arguments.\n"
                       "USAGE: edit <ID> <field> <new value>\n"
                       "Available fields: 'name', 'surname', 'group', "
                       "'record'.\n"
                       "You can get student ID by using 'list' or 'query'.\n";

                return;
            }

            size_t n = cm_parsell(args[1]);

            if (n == COMMON_INVALID_NUMBERLL)
            {
                std::cout << "ERROR: ID is not a number." << std::endl;
                return;
            }

            size_t pos = model.search(n);

            if (pos == MODEL_NOT_FOUND)
            {
                std::cout << "ERROR: Invalid ID." << std::endl;
                return;
            }

            Student &student = model.get_mut_ref(pos);

            std::string value = cli_concat_args(args, 3);

            std::cout << std::quoted(value) << std::endl;

            if (cm_str_tolower(args[2]) == "name")
            {
                student.name = value;
            }
            else if (cm_str_tolower(args[2]) == "surname")
            {
                student.surname = value;
            }
            else if (cm_str_tolower(args[2]) == "group")
            {
                student.group = value;
            }
            else if (cm_str_tolower(args[2]) == "record")
            {
                student.record_book = value;
            }
            else
            {
                std::cout << "ERROR: Unknown field. "
                             "Available fields: 'name', 'surname', 'group', "
                             "'record'."
                          << std::endl;
                return;
            }

            fputc('\n', stdout);

            cli_put_table_header();
            cli_put_student(student);

            std::fflush(stdout);
        }

        break;

        case GRADE: {
            throw std::logic_error("TODO");
        }
        break;

        case CLEAR: {
            std::cout << "Do you really want to do that?\n";
            if (!cli_y_or_n())
                return;

            model.clear();
            std::cout << "Database has been erased." << std::endl;
        }
        break;

        case COMMIT: {
            std::cout << "Saving..." << std::endl;
            model.write_file();
        }
        break;

        case REVERT: {
            std::cout << "Reverting changes..." << std::endl;
            model.reread_file();
        }
        break;
    }
}

void cli_loop(const char *filepath)
{
    int err = std::setvbuf(stdout, NULL, _IOLBF, 1024);

    if (err)
    {
        std::cout << "Could not enable bufferin for output.\nPlease buy more "
                     "memory :c"
                  << std::endl;
        exit(1);
    }

    InMemoryModel *model;

    try
    {
        std::cout << "Opening '" << filepath << "'..." << std::endl;
        model = new InMemoryModel(filepath);
    }
    catch (std::ios::failure &e)
    {
        // iostream error's .what() method returns weird string at the end
        std::cout << filepath << ": Could not open file: " << strerror(errno)
                  << std::endl;
        std::exit(1);
    }
    // Logic errors while opening should be parsing errors.
    catch (std::logic_error &e)
    {
        std::cout << filepath << ": Parsing error: " << e.what() << std::endl;
        exit(1);
    }

    std::string filename = cli_extract_filename(filepath);

    std::cout << "\nWelcome to toiletdb " << TOILET_VERSION << '.' << std::endl;
    std::cout << "Loaded " << model->size()
              << " students.\n"
                 "Try 'help' to see available commands."
              << std::endl;

    while (true)
    {
        std::cout << "\n" << filename << "# ";

        static std::string line;
        std::getline(std::cin, line);

        if (line.empty())
        {
            continue;
        }

        std::vector<std::string> args = cli_split_args(line);

        try
        {
            cli_exec(*model, args);
        }
        // Logic exceptions at execution should be recoverable errors.
        catch (std::logic_error &e)
        {
            std::cout << "Logic error: " << e.what() << std::endl;
        }
        catch (std::ios::failure &e)
        {
            std::cout << "IO error: " << strerror(errno) << std::endl;
            std::exit(1);
        }
        catch (std::runtime_error &e)
        {
            std::cout << "Runtime error: " << e.what() << std::endl;
            std::exit(1);
        }
    }
}
