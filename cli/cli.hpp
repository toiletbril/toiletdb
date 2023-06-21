#ifndef TOILETDB_CLI_H_
#define TOILETDB_CLI_H_

#include <cctype>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>

#include "toiletdb.hpp"

#include "toiletline/toiletline.c"

void cli_loop(const char *filepath);

#endif // TOILETDB_CLI_H_
