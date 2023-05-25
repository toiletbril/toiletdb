#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS
    #include <Windows.h>
#endif

#include <ostream>

// #define DEBUG

#include "cli.cpp"
#include "common.cpp"

/*
    FIXME:
        - Type check numeric fields when parsing
        - File creation when there is no file
        - CRLF parsing for Windows
        - Rewrite all commands
    TODO:
        - ID uniqueness checking.
        - Arrays
*/

static bool help        = false;
static bool help_format = false;

void show_help()
{
    std::cout << "USAGE: " << TOILET_NAME << " <database file>\n"
              << "CLI for manipulating toiletdb files.\n"
                 "\n"
                 "FLAGS:\n"
                 "\t-?, --help\t\tSee this menu.\n"
                 "\t    --help-format\tShow help for database file format.\n"
                 "\n"
                 "v"
              << TOILET_VERSION << " (c) toiletbril " << TOILET_GITHUB
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
                 "\t|<value of column type>| ...\n"
                 "\t...\n"
                 "```\n"
                 "\n"
                 "Available types are:\n"
                 "- `int`   32 bit signed integer\n"
                 "- `b_int` 64 bit unsigned integer\n"
                 "- `str`   Array of 8 bit characters\n"
                 "\n"
                 "Modifiers:\n"
                 "- `const` Marks column as not editable\n"
                 "- `id`    Marks column to be used for indexing "
                 "(available only for type `b_int`)\n"
                 "\n"
                 "Supported format versions: 1\n"
                 "\n"
              << TOILET_VERSION << " (c) toiletbril " << TOILET_GITHUB
              << std::endl;
    exit(0);
}

void set_flag(const char *s)
{
    if (s[0] == '-')
    {
        if (s[1] == '?')
        {
            help = true;
        }
        // Long flags
        else if (s[1] == '-')
        {
            if (strcmp(s, "--help") == 0)
            {
                help = true;
            }
            else if (strcmp(s, "--help-format") == 0)
            {
                help_format = true;
            }
            else
            {
                std::cout << "Unknown flag " << s << ". Try '--help'."
                          << std::endl;
                exit(1);
            }
        }
        else
        {
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

    if (argc < 2)
    {
        std::cout << "ERROR: Not enough arguments.\n"
                  << "USAGE: " << TOILET_NAME
                  << " <database file>\n"
                     "To get more help, try '--help'.\n"
                     "\n"
                  << "v" TOILET_VERSION << " (c) toiletbril " << TOILET_GITHUB
                  << std::endl;
        std::exit(0);
    }

    for (int i = 1; i < argc; ++i)
    {
        set_flag(argv[i]);
    }

    if (help)
    {
        show_help();
    }

    if (help_format)
    {
        show_help_format();
    }

#ifdef DEBUG
    std::cout << "*** Debug is enabled.\n";
#endif

    cli_loop(argv[1]);
    return 0;
}
