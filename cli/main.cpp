#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS
    #include <Windows.h>
#endif

#include <cstring>
#include <iostream>

#include "cli.hpp"
#include "toiletdb.hpp"

static bool flag_help    = false;
static bool flag_format  = false;
static bool flag_version = false;

#define TOILETDB_NAME "toiletdb"
#define TOILETDB_GITHUB "<https://github.com/toiletbril>"

[[noreturn]] void show_help()
{
    std::cout << "USAGE: " << TOILETDB_NAME << " [-options] <database file>\n"
              << "CLI for manipulating toiletdb files.\n"
                 "\n"
                 "OPTIONS:\n"
                 "  -?, --help       \tDisplay this menu.\n"
                 "      --help-format\tDisplay help for database file format.\n"
                 "      --version    \tDisplay version."
              << std::endl;
    exit(0);
}

[[noreturn]] void show_version()
{
    std::cout << "toiletdb " << TOILETDB_VERSION << "\n"
              << "(c) toiletbril " << TOILETDB_GITHUB
              << std::endl;
    exit(0);
}

[[noreturn]] void show_help_format()
{
    std::cout << "Database files look like this:\n"
                 "```\n"
                 "tdb<format version>\n"
                 "|[modifier] <column type> <column name>| ...\n"
                 "|<value of column type>| ...\n"
                 "|<another value of column type>| ...\n"
                 "...\n"
                 "```\n"
                 "\n"
                 "Available types are:\n"
                 "- `int`   Signed integer\n"
                 "- `uint`  Unsigned integer\n"
                 "- `str`   Character string\n"
                 "\n"
                 "Modifiers:\n"
                 "- `const` Marks column as not editable\n"
                 "- `id`    Marks column to be used for indexing "
                 "(available only for `const uint`)\n"
                 "\n"
                 "supported format versions: <="
              << TOILETDB_PARSER_FORMAT_VERSION << "\n"
              << TOILETDB_VERSION << " (c) toiletbril " << TOILETDB_GITHUB
              << std::endl;
    exit(0);
}

void set_flag(const char *s, std::vector<const char *> &args)
{
    if (strlen(s) < 2) {
        return;
    };

    if (s[0] != '-') {
        args.push_back(s);
        return;
    }

    if (s[1] == '?') {
        flag_help = true;
        return;
    }

    // Long flags
    if (s[1] == '-') {
        if (strcmp(s, "--help") == 0) {
            flag_help = true;
            return;
        }
        if (strcmp(s, "--help-format") == 0) {
            flag_format = true;
            return;
        }
        if (strcmp(s, "--version") == 0) {
            flag_version = true;
            return;
        }
        else {
            std::cout << "Unknown flag " << s << ". Try '--help'."
                      << std::endl;
            exit(1);
        }
    }

    else {
        std::cout << "Unknown flag " << s << ". Try '--help'." << std::endl;
        exit(1);
    }
}

int main(int argc, char **argv)
{
#ifndef NDEBUG
    std::cout << "*** Debug is enabled.\n";
#endif

    std::vector<const char *> args;

    for (int i = 1; i < argc; ++i) {
        set_flag(argv[i], args);
    }

    if (flag_help) {
        show_help();
    }

    if (flag_format) {
        show_help_format();
    }

    if (flag_version) {
        show_version();
    }

    TDB_DEBUGV(args, "args");

    if (args.size() < 1) {
        std::cout << "ERROR: Not enough arguments.\n"
                     "To get more help, try '--help'."
                  << std::endl;
        std::exit(0);
    }

    cli_loop(args[0]);
    return 0;
}
