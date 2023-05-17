#ifdef _WIN32
    #define _CRT_SECURE_NO_WARNINGS
    #include <Windows.h>
#endif

#include <ostream>

// #define DEBUG

#define VERSION "0.7"
#define NAME "toiletdb"
#define GITHUB "<https://github.com/toiletbril>"

#include "cli.cpp"

/*
    TODO:
        - Checks for free IDs when adding, so IDs are not skipped
        - Grading system (idk how to implement)
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
        std::cout << "Usage: " << NAME
                  << " <database file>\n"
                     "If there is no such file, it will be created.\n"
                  << "v" VERSION << " (c) toiletbril " << GITHUB << std::endl;
        std::exit(0);
    }

#ifdef DEBUG
    std::cout << "*** Debug is enabled.\n";
#endif

    cli_loop(argv[1]);
    return 0;
}
