#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS
    #include <Windows.h>
#endif

#include <ostream>

#define DEBUG

#include "cli.cpp"
#include "common.cpp"

/*
    FIXME:
        - Type check numeric fields when parsing
        - Word wrapping
        - File creation
        - CRLF parsing
    TODO:
        - ID uniqueness checking.
        - Arrays
*/

int main(int argc, char **argv)
{
#ifdef _WIN32
    // I'm not sure if this does anything.
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);
#endif

    if (argc < 2)
    {
        std::cout << "USAGE: " << TOILET_NAME
                  << " <database file>\n"
                     "If there is no such file, it will be created.\n"
                  << "v" TOILET_VERSION << " (c) toiletbril " << TOILET_GITHUB
                  << std::endl;
        std::exit(0);
    }

#ifdef DEBUG
    std::cout << "*** Debug is enabled.\n";
#endif

    cli_loop(argv[1]);
    return 0;
}
