#!/usr/bin/env python3

# this compresses all .hpp files into one

import glob

files = glob.glob("./src/*.hpp")
files_no_path = [file.split("/")[-1] for file in files]

header = f"/**\n * This header was automatically generated.\n * \n * Files included:\n * {', '.join(files_no_path)} \n */\n\n"
includes = set()

in_header = False

for f in files:
    # move all includes to the top
    with open(f, "r") as file:
        for line in file:
            if line.startswith("#include \""):
                continue
            if line.startswith("#include <"):
                includes.add(line)

header += "".join(includes)

for f in files:
    # make header
    with open(f, "r") as file:
        header += "\n/**\n * File:" + file.name + "\n */\n\n"
        for line in file:
            # skip includes
            if line.startswith("#include"):
                continue
            header += line
    header += "\n"

blank = False
actual_header = ""

# remove two and more blank lines
for line in header.splitlines():
    if not line:
        if blank:
            continue
        blank = True
    else:
        blank = False
    actual_header += line + "\n"

print(actual_header)