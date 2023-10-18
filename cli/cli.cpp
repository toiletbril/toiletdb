#include "cli.hpp"

using namespace toiletdb;

#define CLI_INTW 12
#define CLI_B_INTW 16
#define CLI_STRW 24

#define CLI_MARGIN 4

enum CLI_COMMAND_KIND
{
    UNKNOWN = 0,
    HELP,
    VERSION,
    EXIT,
    EXIT_NO_SAVE,
    QUERY,
    LIST,
    LIST_TYPES,
    DBSIZE,
    ADD,
    REMOVE,
    EDIT,
    CLEAR,
    COMMITAS,
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
// Supports escaping, i. e. "Gorlock \"The Destroyer\""
static std::vector<std::string> cli_split_args(const std::string &s)
{
    size_t len = s.length();

    std::vector<std::string> result;
    std::string temp;

    bool in_quotes = false;
    bool escaped   = false;

    for (size_t i = 0; i < len; ++i) {
        char c = s.at(i);

        if (escaped) {
            temp += c;
            escaped = false;
        }

        else if (c == '\\') {
            escaped = true;
        }

        else if (in_quotes) {
            if (c == '"') {
                in_quotes = false;
            }
            else {
                temp += c;
            }
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

    TDB_DEBUGV(result, "cli_splitstring");

    return result;
}

// Prints a prompt which asks to input Y or N.
static bool cli_y_or_n()
{
    while (true) {
        char buf[2] = {0};

        tl_getc(buf, 2, "    Y or N? ");

        char answer = std::tolower(buf[0]);

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
static void cli_put_table_header(InMemoryTable &model)
{
    std::vector<std::string> names = model.get_column_names();
    std::vector<int> types         = model.get_types();

    size_t len = names.size();

    std::stringstream modifier;
    std::stringstream type;
    std::stringstream name;

    size_t padding = 0;

    for (size_t i = 0; i < len; ++i) {
        if (types[i] & TT_INT) {
            padding = CLI_INTW;
            name << std::left << std::setw(CLI_INTW) << names[i];
        }

        else if (types[i] & TT_UINT) {
            padding = CLI_B_INTW;
            name << std::left << std::setw(CLI_B_INTW) << names[i];
        }

        else if (types[i] & TT_STR) {
            padding = CLI_STRW;
            name << std::left << std::setw(CLI_STRW) << names[i];
        }

        // Output modifiers

        std::string temp;

        if (types[i] & TT_ID) {
            temp += "[id]";
        }

        if (types[i] & TT_CONST) {
            temp += "[const]";
        }

        modifier << std::left << std::setw(padding)
                 << (temp.empty() ? " " : temp);

        temp.clear();

        // Output types

        if (types[i] & TT_INT) {
            temp = "[int]";
        }

        else if (types[i] & TT_UINT) {
            temp = "[uint]";
        }

        else if (types[i] & TT_STR) {
            temp = "[str]";
        }

        type << std::left << std::setw(padding) << temp;
    }

    std::cout << modifier.str() << "\n";
    std::cout << type.str() << "\n";
    std::cout << name.str() << "\n";
}

// Prints out a row.
// Wraps words by breaking them to the next line
// if they exceed column width.
static void cli_put_row(InMemoryTable &model, const size_t &pos)
{
    std::stringstream wrap_buf;

    bool should_wrap = false;

    size_t line = 1;

    std::vector<int> types  = model.get_types();
    std::vector<void *> row = model.unsafe_get_mut_row(pos);

    size_t len = types.size();

    for (size_t i = 0; i < len; ++i) {
        switch (TDB_TYPE(types[i])) {
            case TT_INT: {
                // Int should always fit.
                continue;
            } break;

            case TT_UINT: {
                if (TDB_CAST(size_t, row[i]) >= 100000000000) {
                    should_wrap = true;
                    break;
                }
            } break;

            case TT_STR: {
                if (TDB_CAST(std::string, row[i]).size() > CLI_STRW - CLI_MARGIN) {
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
            switch (types[i] & TDB_TMASK) {
                case TT_INT: {
                    std::cout << std::left << std::setw(CLI_INTW)
                              << TDB_CAST(int, row[i]);
                } break;

                case TT_UINT: {
                    std::cout << std::left << std::setw(CLI_B_INTW)
                              << TDB_CAST(size_t, row[i]);
                } break;

                case TT_STR: {
                    std::cout << std::left << std::setw(CLI_STRW)
                              << TDB_CAST(std::string, row[i]);
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
            switch (types[i] & TDB_TMASK) {
                case TT_INT: {
                    // Int always should fit
                    int n         = TDB_CAST(int, row[i]);
                    std::string s = std::to_string(n);

                    cols.push_back(std::to_string(n));
                    lengths.push_back(s.size());
                } break;

                case TT_UINT: {
                    size_t n = TDB_CAST(size_t, row[i]);

                    std::string s = std::to_string(n);

                    cols.push_back(std::to_string(n));
                    lengths.push_back(s.size());
                } break;

                case TT_STR: {
                    std::string s = TDB_CAST(std::string, row[i]);

                    cols.push_back(s);
                    lengths.push_back(tl_utf8_strlen(s.c_str()));
                } break;

                default:
                    std::logic_error("Unreachable");
            }
        }

        TDB_DEBUGV(lengths, "lengths");

        while (should_wrap) {
            if (line > 1) {
                wrap_buf << "\n";
            }

            // Slice very long string into lines.
            // Can be several lines long.

            for (size_t i = 0; i < len; ++i) {
                switch (types[i] & TDB_TMASK) {
                    case TT_INT: {
                        wrap_buf << std::left << std::setw(CLI_INTW) << ' ';
                    } break;

                    case TT_UINT: {
                        wrap_buf << std::left << std::setw(CLI_B_INTW)
                                 << (lengths[i] > (CLI_B_INTW - CLI_MARGIN)
                                         ? cols[i].substr(
                                               (CLI_B_INTW - CLI_MARGIN) * line,
                                               CLI_B_INTW - CLI_MARGIN)
                                         : " ");

                        lengths[i] -= CLI_B_INTW;
                    } break;

                    case TT_STR: {
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
                switch (types[i] & TDB_TMASK) {
                    case TT_INT: {
                        continue;
                    } break;

                    case TT_UINT: {
                        should_wrap =
                            should_wrap || lengths[i] > CLI_B_INTW - CLI_MARGIN;
                    } break;

                    case TT_STR: {
                        should_wrap =
                            should_wrap || lengths[i] > CLI_STRW - CLI_MARGIN;
                    } break;
                }
            }

            ++line;
        }

        for (size_t i = 0; i < len; ++i) {
            switch (TDB_TYPE(types[i])) {
                case TT_INT: {
                    std::cout << std::left << std::setw(CLI_INTW) << cols[i];
                } break;

                case TT_UINT: {
                    std::cout << std::left << std::setw(CLI_B_INTW)
                              << cols[i].substr(0, CLI_B_INTW - CLI_MARGIN);
                } break;

                case TT_STR: {
                    std::cout << std::left << std::setw(CLI_STRW)
                              << cols[i].substr(0, CLI_STRW - CLI_MARGIN);
                } break;

                default:
                    std::logic_error("Unreachable");
            }
        }

        std::cout << '\n'
                  << wrap_buf.str() << '\n';
    }
}

static CLI_COMMAND_KIND cli_get_command(std::string &s)
{
    if (s == "help" || s == "?")
        return HELP;
    if (s == "version" || s == "ver" || s == "v")
        return VERSION;
    if (s == "exit" || s == "quit" || s == "q")
        return EXIT;
    if (s == "exit!" || s == "quit!" || s == "q!")
        return EXIT_NO_SAVE;
    if (s == "search" || s == "s")
        return QUERY;
    if (s == "list" || s == "ls")
        return LIST;
    if (s == "types" || s == "lst")
        return LIST_TYPES;
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
    if (s == "commitas" || s == "saveas")
        return COMMITAS;
    if (s == "commit" || s == "save")
        return COMMIT;
    if (s == "revert" || s == "reset")
        return REVERT;

    return UNKNOWN;
}

static int cli_exec(InMemoryTable &model, std::vector<std::string> &args)
{
    CLI_COMMAND_KIND c;

    if (args.size() > 0)
        c = cli_get_command(args[0]);
    else
        return 0;

    switch (c) {
        case UNKNOWN: {
            std::cout << "ERROR: Unknown command "
                      << "'" << args[0] << "'.\n"
                      << "Try 'help' to see available commands."
                      << std::endl;
        } break;

        case HELP: {
            std::cout << "Available commands:\n"
                         "    help, ?             See this message.\n"
                         "    version, ver        Display version.\n"
                         "    exit, quit, q       Save and quit. Append '!' to the end to skip saving.\n"
                         "    search, s           Search the database.\n"
                         "    list, ls            Show all rows.\n"
                         "    types, lst          Show only a table header.\n"
                         "    size                See total amount of rows in database.\n"
                         "    add                 Add a row to database.\n"
                         "    remove, rm          Remove a row from database.\n"
                         "    edit, e             Edit a row.\n"
                         "    clear               Clear the database.\n"
                         "    commitas, saveas    Save changes to the file specified.\n"
                         "    commit, save        Save changes.\n"
                         "    revert, reset       Revert uncommited changes."
                      << std::endl;
        } break;

        case VERSION: {
            std::cout << "toiletdb " << TOILETDB_VERSION << "\n"
                      << "supported format versions: <= " << TOILETDB_PARSER_FORMAT_VERSION
                      << std::endl;
        } break;

        case EXIT: {
            std::cout << "Saving..." << std::endl;
            model.write_file();
            std::cout << "Exiting..." << std::endl;

            return 1;
        } break;

        case EXIT_NO_SAVE: {
            std::cout << "Exiting..." << std::endl;

            return 1;
        } break;

        case LIST: {
            size_t len = model.get_row_count();

            if (len > 1000) {
                std::cout << "Database has over 1 000 entries "
                             "(" << len << ").\n"
                             "This can take a long time.\n"
                             "Do you really want to list them all?"
                          << std::endl;

                if (!cli_y_or_n()) {
                    return 0;
                }
            }

            cli_put_table_header(model);

            for (size_t i = 0; i < len; ++i) {
                cli_put_row(model, i);
            }

            std::fflush(stdout);
        } break;

        case LIST_TYPES: {
            cli_put_table_header(model);
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

                std::cout
                    << "ERROR: Not enough arguments.\n"
                       "Usage: search <field> <value>\n"
                       "Available fields: "
                    << fields
                    << "\n"
                       "For more information on column types, use 'types'."
                    << std::endl;

                return 0;
            }

            std::string query = cli_concat_args(args, 2);
            size_t column_pos = model.search_column_index(args[1]);

            if (column_pos == TDB_NOT_FOUND) {
                std::cout << "ERROR: Unknown column '" << args[1] << "'."
                          << std::endl;
                return 0;
            }

            // If column specified has modifier 'id', use binary search.
            if (model.get_column_type(column_pos) & TT_ID) {
                size_t value = parse_long_long(query);

                if (value == TDB_INVALID_ULL) {
                    std::cout << "ERROR: ID is not a number." << std::endl;
                    return 0;
                }

                size_t pos = model.search(value);

                cli_put_table_header(model);

                if (pos != TDB_NOT_FOUND) {
                    cli_put_row(model, model.search(value));
                }

                std::fflush(stdout);

                return 0;
            };

            std::vector<size_t> positions = model.search(args[1], query);

            cli_put_table_header(model);

            for (const size_t &pos : positions) {
                cli_put_row(model, pos);
            }

            std::fflush(stdout);
        } break;

        case DBSIZE: {
            std::cout << "There are " << model.get_row_count()
                      << " rows in database." << std::endl;
        } break;

        case ADD: {
            size_t len             = model.get_column_count();
            std::vector<int> types = model.get_types();

            if (args.size() - 1 != model.get_column_count() - 1) {
                std::string fields;
                std::vector<std::string> names = model.get_column_names();

                for (size_t i = 0; i < len - 1; ++i) {
                    // ID field will be added automatically.
                    if (types[i] & TT_ID) {
                        continue;
                    }
                    fields += "<" + names[i] + "> ";
                }
                fields += "<" + names[len - 1] + ">";

                std::cout
                    << "ERROR: Invalid number of arguments. "
                    << "(" << len - 1 << " needed, actual " << args.size() - 1 << ")\n"
                    << "Usage: add_row " << fields << "\n"
                       "You can put quotes around fields.\n"
                       "EXAMPLE: Vasiliy \"Ivanov Petrov\" \"Very "
                       "Cool\" 69420\n"
                       "For more information on column types, use 'types'."
                    << std::endl;
                return 0;
            }

            // Remove the command from vector, so only values remain
            args.erase(args.begin());

            char errc = cli_forbiddenchar(args);

            if (errc) {
                std::cout << "ERROR: Forbidden character '" << errc << "'."
                          << std::endl;
                return 0;
            }

            TDB_DEBUGV(args, "add_row() args");

            int err = model.add_row(args);

            if (!err) {
                cli_put_table_header(model);
                cli_put_row(model, model.get_row_count() - 1);
            }
            else {
                std::cout << "ERROR: ";
                if (err == 1) {
                    std::cout << "Wrong number of arguments." << std::endl;
                }
                else if (err == 2 || err == 3) {
                    std::cout << "Invalid value for numeric field."
                              << std::endl;
                }
            }
        } break;

        case REMOVE: {
            if (args.size() != 2) {
                std::cout << "ERROR: Invalid number of arguments.\n"
                             "Usage: remove <ID>\n"
                             "You can get ID by using 'list' or 'search'."
                          << std::endl;
                return 0;
            }

            size_t n = parse_long_long(args[1]);

            if (n == TDB_INVALID_ULL) {
                std::cout << "ERROR: Invalid ID." << std::endl;
                return 0;
            }

            size_t pos = model.search(n);

            if (pos == TDB_NOT_FOUND) {
                std::cout << "ERROR: Could not find row with specified ID."
                          << std::endl;
                return 0;
            }

            cli_put_table_header(model);
            cli_put_row(model, pos);

            model.erase(pos);
        } break;

        case EDIT: {
            std::vector<int> types = model.get_types();

            if (args.size() < 4) {
                std::string fields;
                size_t len                     = model.get_column_count();
                std::vector<std::string> names = model.get_column_names();

                for (size_t i = 0; i < len - 1; ++i) {
                    // ID field will be added automatically
                    if (TDB_IS(types[i], TT_CONST)) {
                        continue;
                    }
                    fields += "'" + names[i] + "' ";
                }
                if (!(TDB_IS(types[len - 1], TT_CONST))) {
                    fields += "'" + names[len - 1] + "'";
                }

                std::cout << "ERROR: Not enough arguments.\n"
                             "Usage: edit <ID> <field> <value>\n"
                             "Available fields: "
                          << fields << "\n";

                return 0;
            }

            size_t n = parse_long_long(args[1]);

            if (n == TDB_INVALID_ULL) {
                std::cout << "ERROR: ID is not a number." << std::endl;
                return 0;
            }

            size_t pos = model.search(n);

            if (pos == TDB_NOT_FOUND) {
                std::cout << "ERROR: Could not find row with specified ID."
                          << std::endl;
                return 0;
            }

            size_t column_index = model.search_column_index(args[2]);

            if (column_index == TDB_NOT_FOUND) {
                std::cout << "Invalid field '" << args[2] << "'" << std::endl;
                return 0;
            }

            std::string value = cli_concat_args(args, 3);

            char errc = cli_forbiddenchar(value);

            if (errc) {
                std::cout << "ERROR: Forbidden character '" << errc << "'."
                          << std::endl;
                return 0;
            }

            void *data = (model.unsafe_get_mut_row(pos))[column_index];

            if (TDB_IS(types[column_index], TT_CONST)) {
                std::cout << "ERROR: Can not edit value with 'const' modifier."
                          << std::endl;
                return 0;
            }

            switch (TDB_TYPE(types[column_index])) {
                case TT_INT: {
                    int number = parse_int(value);

                    if (n == TDB_INVALID_I) {
                        std::cout << "ERROR: Value is not a number."
                                  << std::endl;
                        return 0;
                    }

                    TDB_CAST(int, data) = number;
                } break;

                case TT_UINT: {
                    size_t number = parse_long_long(value);

                    if (n == TDB_INVALID_ULL) {
                        std::cout << "ERROR: Value is not a number."
                                  << std::endl;
                        return 0;
                    }

                    TDB_CAST(size_t, data) = number;
                } break;

                case TT_STR: {
                    TDB_CAST(std::string, data) = value;
                } break;
            }

            cli_put_table_header(model);
            cli_put_row(model, pos);
        }

        break;

        case CLEAR: {
            std::cout << "Do you really want to do that?" << std::endl;
            if (!cli_y_or_n())
                return 0;

            model.clear();
            std::cout << "Database has been erased." << std::endl;
        } break;

        case COMMIT: {
            std::cout << "Saving..." << std::endl;
            model.write_file();
        } break;

        case COMMITAS: {
            if (args.size() != 2) {
                std::cout << "ERROR: Invalid number of arguments.\n"
                             "Usage: commitas <file path>"
                          << std::endl;
                return 0;
            }

            std::cout << "Saving..." << std::endl;
            try {
                model.write_file(args[1]);
            }
            catch (std::logic_error &err) {
                std::cout << "ERROR: File already exists." << std::endl;
            }
        } break;

        case REVERT: {
            std::cout << "Reverting changes..." << std::endl;
            model.reread_file();
        } break;
    }

    return 0;
}

#define LINE_BUF_SIZE 256

int cli_loop(const std::string &filepath)
{
    int err = std::setvbuf(stdout, NULL, _IOLBF, 256);

    if (err) {
        std::cout << "Could not enable bufferin for output.\n"
                     "Please buy more memory :c"
                  << std::endl;
        return 1;
    }

    std::unique_ptr<InMemoryTable> model;

    try {
        std::cout << "Opening '" << filepath << "'..." << std::endl;
        model = std::make_unique<InMemoryTable>(filepath);
    }
    catch (std::ios::failure &e) {
        // iostream error's .what() method returns weird string at the end
        std::cout << filepath << ": Could not open file: " << strerror(errno)
                  << std::endl;
        return 1;
    }
    catch (ParsingError &e) {
        std::cout << filepath << ": " << e.what() << "\n"
                     "Try '--help-format' to see help for database format."
                  << std::endl;
        return 1;
    }
    catch (std::runtime_error &e) {
        TDB_DEBUGS(e.what(), "cli_loop");
        std::cout << filepath << ": " << strerror(errno) << "\n"
                     "Try '--help-format' to see help for database format."
                  << std::endl;
        return 1;
    }

    std::string filename = cli_extract_filename(filepath);

    std::cout << "\nWelcome to toiletdb " << TOILETDB_VERSION << '.'
              << std::endl;
    std::cout << "Loaded " << model->get_row_count() << " rows.\n"
                 "Try 'help' to see available commands."
              << std::endl;

    tl_init();

    std::string line;
    std::string prompt = filename + "# ";

    int code = 0;
    char lb[LINE_BUF_SIZE];

    while (true) {
        std::fflush(stdout);

        int tl_code = tl_readline(lb, LINE_BUF_SIZE, prompt.c_str());
        std::cout << std::endl;

        if (tl_code == TL_PRESSED_INTERRUPT) {
            std::cout << "Interrupted." << std::endl;
            break;
        }

        if (tl_code > 0) {
            code = tl_code;
            break;
        }

        line = lb;

        if (line.empty()) {
            continue;
        }

        std::vector<std::string> args = cli_split_args(line);

        try {
            if (cli_exec(*model, args))
                break;
        }
        // Logic exceptions at execution should be recoverable errors.
        catch (std::logic_error &e) {
            std::cout << "Logic error: " << e.what() << std::endl;
        }
        catch (std::ios::failure &e) {
            std::cout << "IO error: " << strerror(errno) << std::endl;
            code = 1;
        }
        catch (std::runtime_error &e) {
            std::cout << "Runtime error: " << e.what() << std::endl;
            tl_exit();
            code = 1;
        }
    }

    tl_exit();

    return code;
}
