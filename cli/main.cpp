#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS
    #include <Windows.h>
#endif

#include <cstring>
#include <iostream>

#include "cli.hpp"
#include "toiletdb.hpp"

static bool help        = false;
static bool help_format = false;

#define TOILETDB_NAME "toiletdb"
#define TOILETDB_GITHUB "<https://github.com/toiletbril>"

void show_help()
{
    std::cout << "USAGE: " << TOILETDB_NAME << " <database file>\n"
              << "CLI for manipulating toiletdb files.\n"
                 "\n"
                 "FLAGS:\n"
                 "\t-?, --help\t\tSee this menu.\n"
                 "\t    --help-format\tShow help for database file format.\n"
                 "\n"
                 "v"
              << TOILETDB_VERSION << " (c) toiletbril " << TOILETDB_GITHUB
              << std::endl;
    exit(0);
}

void show_help_format()
{
    std::cout << "Database files look like this:\n"
                 "```\n"
                 "\ttdb<format version>\n"
                 "\t|[modifier] <column type> <column name>| ...\n"
                 "\t|<value of column type>| ...\n"
                 "\t|<another value of column type>| ...\n"
                 "\t...\n"
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
                 "Supported format versions: 1\n"
                 "\n"
              << TOILETDB_VERSION << " (c) toiletbril " << TOILETDB_GITHUB
              << std::endl;
    exit(0);
}

void set_flag(const char *s)
{
    if (s[0] == '-') {
        if (s[1] == '?') {
            help = true;
        }
        // Long flags
        else if (s[1] == '-') {
            if (strcmp(s, "--help") == 0) {
                help = true;
            }
            else if (strcmp(s, "--help-format") == 0) {
                help_format = true;
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
}

int main(int argc, char **argv)
{
#ifdef _WIN32
    // I'm not sure if this does anything.
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
#endif

#ifndef NDEBUG
    std::cout << "*** Debug is enabled.\n";
#endif

    if (argc < 2) {
        std::cout << "ERROR: Not enough arguments.\n"
                  << "USAGE: " << TOILETDB_NAME
                  << " <database file>\n"
                     "To get more help, try '--help'."
                  << std::endl;
        std::exit(0);
    }

    for (int i = 1; i < argc; ++i) {
        set_flag(argv[i]);
    }

    if (help) {
        show_help();
    }

    if (help_format) {
        show_help_format();
    }

    cli_loop(argv[1]);
    return 0;
}
