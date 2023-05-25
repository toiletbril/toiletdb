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

#define CLI_INTW 12
#define CLI_B_INTW 16
#define CLI_STRW 24

#define CLI_MARGIN 4

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

    for (size_t i = pos + 1; i < len; ++i) {
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

    for (size_t i = 0; i < len; ++i) {
        char c = s.at(i);

        if (in_quotes) {
            if (c == '"') {
                in_quotes = false;
                continue;
            }
            temp += c;
        }
        else if (c == '"') {
            in_quotes = true;
        }
        else if (c == ' ') {
            if (!temp.empty()) {
                result.push_back(temp);
            }
            temp.clear();
        }
        else {
            temp += c;
        }
    }
    if (!temp.empty()) {
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
    while (true) {
        std::cout << "\tY or N >> ";
        std::fflush(stdout);

        char answer = std::tolower(std::fgetc(stdin));

        // Discard the rest of the line.
        while (fgetc(stdin) != '\n') {}

        if (answer == 'y') {
            return true;
        }
        else if (answer == 'n') {
            return false;
        }
        else {
            std::cout << "ERROR: Invalid character." << std::endl;
        }
    }

    return false;
}

#define CLI_FORBIDDEN_CHARS "|[]"

// Returns first invalid character it finds.
static char cli_forbiddenchar(std::vector<std::string> &args)
{
    std::string forbidden_chars = CLI_FORBIDDEN_CHARS;

    for (const std::string &a : args) {
        for (char const &c : a) {
            if (forbidden_chars.find(c) != std::string::npos) {
                return c;
            }
        }
    }
    return 0;
}

static char cli_forbiddenchar(std::string &arg)
{
    std::string forbidden_chars = CLI_FORBIDDEN_CHARS;

    for (char const &c : arg) {
        if (forbidden_chars.find(c) != std::string::npos) {
            return c;
        }
    }
    return 0;
}

// Puts modifiers first, then names of columns.
static void cli_put_table_header(InMemoryModel &model)
{
    std::vector<std::string> names = model.get_column_names();
    std::vector<int> types         = model.get_column_types();

    size_t len = names.size();

    std::stringstream type;
    std::stringstream modifier;

    size_t padding = 0;

    for (size_t i = 0; i < len; ++i) {
        if (types[i] & FINT) {
            padding = CLI_INTW;
            type << std::left << std::setw(CLI_INTW) << names[i];
        }
        else if (types[i] & FB_INT) {
            padding = CLI_B_INTW;
            type << std::left << std::setw(CLI_B_INTW) << names[i];
        }
        else if (types[i] & FSTR) {
            padding = CLI_STRW;
            type << std::left << std::setw(CLI_STRW) << names[i];
        }

        std::string temp;

        if (types[i] & FID) {
            temp += "[ID]";
        }

        if (types[i] & FCONST) {
            temp += "[CONST]";
        }

        modifier << std::left << std::setw(padding)
                 << (temp.empty() ? " " : temp);
    }

    std::cout << modifier.str() << "\n";
    std::cout << type.str() << "\n";
}

// Prints out a row.
// Wraps words by breaking them to the next line
// if they exceed column width.
static void cli_put_row(InMemoryModel &model, const size_t &pos)
{
    std::stringstream wrap_buf;

    bool should_wrap = false;

    size_t line = 1;

    std::vector<int> types  = model.get_column_types();
    std::vector<void *> row = model.get_row(pos);

    size_t len = types.size();

    for (size_t i = 0; i < len; ++i) {
        switch (types[i] & PARSER_TYPE_MASK) {
            case FINT: {
                // Int should always fit.
                continue;
            } break;

            case FB_INT: {
                if (*static_cast<unsigned long long *>(row[i]) >=
                    100000000000) {
                    should_wrap = true;
                    break;
                }
            } break;

            case FSTR: {
                if (static_cast<std::string *>(row[i])->size() >
                    CLI_STRW - CLI_MARGIN) {
                    should_wrap = true;
                    break;
                }
            } break;

            default:
                std::logic_error("Unreachable");
        }
    }

    if (!should_wrap) {
        for (size_t i = 0; i < len; ++i) {
            switch (types[i] & PARSER_TYPE_MASK) {
                case FINT: {
                    std::cout << std::left << std::setw(CLI_INTW)
                              << *static_cast<int *>(row[i]);
                } break;

                case FB_INT: {
                    std::cout << std::left << std::setw(CLI_B_INTW)
                              << *static_cast<unsigned long long *>(row[i]);
                } break;

                case FSTR: {
                    std::cout << std::left << std::setw(CLI_STRW)
                              << *static_cast<std::string *>(row[i]);
                } break;

                default:
                    std::logic_error("Unreachable");
            }
        }
        std::cout << '\n';
    }
    else {
        std::vector<int> lengths;
        std::vector<std::string> cols;

        if (line > 1) {
            wrap_buf << '\n';
        }

        for (size_t i = 0; i < len; ++i) {
            switch (types[i] & PARSER_TYPE_MASK) {
                case FINT: {
                    // Int always should fit
                    int n         = *static_cast<int *>(row[i]);
                    std::string s = std::to_string(n);

                    cols.push_back(std::to_string(n));
                    lengths.push_back(s.size());
                } break;

                case FB_INT: {
                    unsigned long long n =
                        *static_cast<unsigned long long *>(row[i]);

                    std::string s = std::to_string(n);

                    cols.push_back(std::to_string(n));
                    lengths.push_back(s.size());
                } break;

                case FSTR: {
                    std::string s = *static_cast<std::string *>(row[i]);

                    cols.push_back(s);
                    lengths.push_back(s.size());
                } break;

                default:
                    std::logic_error("Unreachable");
            }
        }

        while (should_wrap) {
            if (line > 1) {
                wrap_buf << "\n";
            }

            // Slice very long string into lines.
            // Can be several lines long.

            for (size_t i = 0; i < len; ++i) {
                switch (types[i] & PARSER_TYPE_MASK) {
                    case FINT: {
                        wrap_buf << std::left << std::setw(CLI_INTW) << ' ';
                    } break;

                    case FB_INT: {
                        wrap_buf << std::left << std::setw(CLI_B_INTW)
                                 << (lengths[i] > (CLI_B_INTW - CLI_MARGIN)
                                         ? cols[i].substr(
                                               (CLI_B_INTW - CLI_MARGIN) * line,
                                               CLI_B_INTW - CLI_MARGIN)
                                         : " ");

                        lengths[i] -= CLI_B_INTW;
                    } break;

                    case FSTR: {
                        wrap_buf << std::left << std::setw(CLI_STRW)
                                 << (lengths[i] > (CLI_STRW - CLI_MARGIN)
                                         ? cols[i].substr(
                                               (CLI_STRW - CLI_MARGIN) * line,
                                               CLI_STRW - CLI_MARGIN)
                                         : " ");

                        lengths[i] -= CLI_STRW;
                    } break;
                }
            }

            // Recheck if any column still exceeds width.

            should_wrap = false;

            for (size_t i = 0; i < len; ++i) {
                switch (types[i] & PARSER_TYPE_MASK) {
                    case FINT: {
                        continue;
                    } break;

                    case FB_INT: {
                        should_wrap =
                            should_wrap || lengths[i] > CLI_B_INTW - CLI_MARGIN;
                    } break;

                    case FSTR: {
                        should_wrap =
                            should_wrap || lengths[i] > CLI_STRW - CLI_MARGIN;
                    } break;
                }
            }

            ++line;
        }

        for (size_t i = 0; i < len; ++i) {
            switch (types[i] & PARSER_TYPE_MASK) {
                case FINT: {
                    std::cout << std::left << std::setw(CLI_INTW) << cols[i];
                } break;

                case FB_INT: {
                    std::cout << std::left << std::setw(CLI_B_INTW)
                              << cols[i].substr(0, CLI_B_INTW - CLI_MARGIN);
                } break;

                case FSTR: {
                    std::cout << std::left << std::setw(CLI_STRW)
                              << cols[i].substr(0, CLI_STRW - CLI_MARGIN);
                } break;

                default:
                    std::logic_error("Unreachable");
            }
        }

        std::cout << '\n' << wrap_buf.str() << '\n';
    }
}

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

static void cli_exec(InMemoryModel &model, std::vector<std::string> &args)
{
    CLI_COMMAND_KIND c = cli_getcommand(args[0]);

    switch (c) {
        case UNKNOWN: {
            std::cout << "ERROR: Unknown command '" << args[0]
                      << "'.\nTry 'help' to see available commands."
                      << std::endl;
        } break;

        case HELP: {
            std::cout << "Available commands:\n"
                         "\thelp  \t?\t\tSee this message.\n"
                         "\texit  \tq, quit\t\tSave and quit. "
                         "Append '!' to the end to skip saving.\n"
                         "\tsearch\ts\t\tSearch the database.\n"
                         "\tlist  \tls\t\tList all rows.\n"
                         "\tsize  \t\t\tSee total amount of rows in database.\n"
                         "\tadd   \t\t\tAdd a row to database.\n"
                         "\tremove\trm\t\tRemove a row from database.\n"
                         "\tedit  \te\t\tEdit a row.\n"
                         "\tclear \t\t\tClear the database.\n"
                         "\tcommit\tsave\t\tSave changes to the file.\n"
                         "\trevert\treverse\t\tRevert uncommited changes."
                      << std::endl;
        } break;

        case EXIT: {
            std::cout << "Saving..." << std::endl;
            model.write_file();
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

            cli_put_table_header(model);

            for (size_t i = 0; i < len; ++i) {
                cli_put_row(model, i);
            }

            std::fflush(stdout);
        } break;

        case QUERY: {
            if (args.size() < 3) {
                size_t len                     = model.get_column_count();
                std::vector<std::string> names = model.get_column_names();

                std::string fields;

                for (size_t i = 0; i < len - 1; ++i) {
                    fields += "'" + names[i] + "', ";
                }
                fields += "'" + names[len - 1] + "'";

                std::cout << "ERROR: Not enough arguments.\n"
                             "Usage: search <field> <value>\n"
                             "Available fields: "
                          << fields << "\n";

                return;
            }

            std::string query = cli_concat_args(args, 2);

            // If column specified has modifier 'id', use binary search.
            if (model.get_column_types()[model.search_column_index(args[1])] & FID) {
                size_t value = cm_parsell(query);

                if (value == COMMON_INVALID_NUMBERLL) {
                    std::cout << "ERROR: ID is not a number." << std::endl;
                    return;
                }

                size_t pos = model.search(value);

                cli_put_table_header(model);

                if (pos != MODEL_NOT_FOUND) {
                    cli_put_row(model, model.search(value));
                }

                std::fflush(stdout);

                return;
            };

            std::vector<size_t> positions = model.search(args[1], query);

            cli_put_table_header(model);
            for (const size_t &pos : positions) {
                cli_put_row(model, pos);
            }

            std::fflush(stdout);
        } break;

        case DBSIZE: {
            std::cout << "There are " << model.size() << " rows in database."
                      << std::endl;
        } break;

        case ADD: {
            size_t len             = model.get_column_count();
            std::vector<int> types = model.get_column_types();

            if (args.size() - 1 != model.get_column_count() - 1) {
                std::string fields;
                std::vector<std::string> names = model.get_column_names();

                for (size_t i = 0; i < len - 1; ++i) {
                    // ID field will be added automatically.
                    if (types[i] & FID) {
                        continue;
                    }
                    fields += "<" + names[i] + "> ";
                }
                fields += "<" + names[len - 1] + ">";

                std::cout << "ERROR: Invalid number of arguments. "
                          << "(" << len - 1 << " needed, actual "
                          << args.size() - 1 << ")\n"
                          << "Usage: add " << fields
                          << "\n"
                             "You can put quotes around fields.\n"
                             "EXAMPLE: Vasiliy \"Ivanov Petrov\" \"Very "
                             "Cool\" 69420"
                          << std::endl;
                return;
            }

            // Remove the command from vector, so only values remain
            args.erase(args.begin());

            char errc = cli_forbiddenchar(args);

            if (errc) {
                std::cout << "ERROR: Forbidden character '" << errc << "'."
                          << std::endl;
                return;
            }

#ifdef DEBUG
            debug_putv(args, "add() args");
#endif

            bool result = model.add(args);

            if (result) {
                std::cout << "Row added successfully." << std::endl;
            }
            else {
                std::cout << "ERROR" << std::endl;
            }
        } break;

        case REMOVE: {
            if (args.size() != 2) {
                std::cout << "ERROR: Invalid number of arguments.\n"
                             "Usage: remove <ID>\n"
                             "You can get ID by using 'list' or 'search'."
                          << std::endl;
                return;
            }

            size_t n = cm_parsell(args[1]);

            if (n == COMMON_INVALID_NUMBERLL) {
                std::cout << "ERROR: Invalid ID." << std::endl;
                return;
            }

            bool success = model.erase_id(n);

            if (success)
                std::cout << "Row removed successfully." << std::endl;
            else
                std::cout << "Could not find row with specified ID."
                          << std::endl;
        } break;

        case EDIT: {
            std::vector<int> types = model.get_column_types();

            if (args.size() < 4) {
                std::string fields;
                size_t len                     = model.get_column_count();
                std::vector<std::string> names = model.get_column_names();

                for (size_t i = 0; i < len - 1; ++i) {
                    // ID field will be added automatically
                    if (types[i] & FCONST) {
                        continue;
                    }
                    fields += "'" + names[i] + "' ";
                }
                if (!(types[len - 1] & FCONST)) {
                    fields += "'" + names[len - 1] + "'";
                }

                std::cout << "ERROR: Not enough arguments.\n"
                             "Usage: edit <ID> <field> <value>\n"
                             "Available fields: "
                          << fields << "\n";

                return;
            }

            size_t n = cm_parsell(args[1]);

            if (n == COMMON_INVALID_NUMBERLL) {
                std::cout << "ERROR: ID is not a number." << std::endl;
                return;
            }

            size_t pos = model.search(n);

            if (pos == MODEL_NOT_FOUND) {
                std::cout << "ERROR: Invalid ID." << std::endl;
                return;
            }

            size_t column_index = model.search_column_index(args[2]);

            if (column_index == MODEL_NOT_FOUND) {
                std::cout << "Invalid field '" << args[2] << "'" << std::endl;
            }

            std::string value = cli_concat_args(args, 3);

            char errc = cli_forbiddenchar(value);

            if (errc) {
                std::cout << "ERROR: Forbidden character '" << errc << "'."
                          << std::endl;
                return;
            }

            void *data = (model.get_row(pos))[column_index];

            if (types[column_index] & FCONST) {
                std::cout << "ERROR: Can not edit value with 'const' modifier."
                          << std::endl;

                return;
            }

            switch (types[column_index] & PARSER_TYPE_MASK) {
                case FINT: {
                    int number = cm_parsei(value);

                    if (n == COMMON_INVALID_NUMBERI) {
                        std::cout << "ERROR: Value is not a number."
                                  << std::endl;
                        return;
                    }

                    *static_cast<int *>(data) = number;
                } break;

                case FB_INT: {
                    unsigned long long number = cm_parsell(value);

                    if (n == COMMON_INVALID_NUMBERLL) {
                        std::cout << "ERROR: Value is not a number."
                                  << std::endl;
                        return;
                    }

                    *static_cast<unsigned long long *>(data) = number;
                } break;

                case FSTR: {
                    *static_cast<std::string *>(data) = value;
                } break;
            }

            cli_put_table_header(model);
            cli_put_row(model, pos);
        }

        break;

        case CLEAR: {
            std::cout << "Do you really want to do that?\n";
            if (!cli_y_or_n())
                return;

            model.clear();
            std::cout << "Database has been erased." << std::endl;
        } break;

        case COMMIT: {
            std::cout << "Saving..." << std::endl;
            model.write_file();
        } break;

        case REVERT: {
            std::cout << "Reverting changes..." << std::endl;
            model.reread_file();
        } break;
    }
}

void cli_loop(const char *filepath)
{
    int err = std::setvbuf(stdout, NULL, _IOLBF, 1024);

    if (err) {
        std::cout << "Could not enable bufferin for output.\nPlease buy more "
                     "memory :c"
                  << std::endl;
        exit(1);
    }

    InMemoryModel *model;

    try {
        std::cout << "Opening '" << filepath << "'..." << std::endl;
        model = new InMemoryModel(filepath);
    }
    catch (std::ios::failure &e) {
        // iostream error's .what() method returns weird string at the end
        std::cout << filepath << ": Could not open file: " << strerror(errno)
                  << std::endl;
        std::exit(1);
    }
    // Logic errors while opening should be parsing errors.
    catch (std::logic_error &e) {
        std::cout << filepath << ": Parsing error: " << e.what()
                  << "\n"
                     "Try '--help-format' to see help for database format."
                  << std::endl;
        exit(1);
    }
    catch (std::runtime_error &e) {
#ifdef DEBUG
        debug_puts(e.what(), "cli_loop");
#endif
        std::cout << filepath << ": File does not exist."
                  << "\n"
                     "You need to create database file first.\n"
                     "Try '--help-format' to see help for database format."
                  << std::endl;
        exit(1);
    }

    std::string filename = cli_extract_filename(filepath);

    std::cout << "\nWelcome to toiletdb " << TOILET_VERSION << '.' << std::endl;
    std::cout << "Loaded " << model->size()
              << " rows.\n"
                 "Try 'help' to see available commands."
              << std::endl;

    while (true) {
        std::cout << "\n" << filename << "# ";

        static std::string line;
        std::getline(std::cin, line);

        if (line.empty()) {
            continue;
        }

        std::vector<std::string> args = cli_split_args(line);

        try {
            cli_exec(*model, args);
        }
        // Logic exceptions at execution should be recoverable errors.
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
