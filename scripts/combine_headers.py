#!/usr/bin/env python3

from re import compile
from sys import argv
from glob import glob
from os import path

# combine headers means:
# - read every header file recursively into a buffer;
# - remove all local includes;
# - move non-local includes to the top;
# - OPTIONAL: strip old guards;
# - OPTIONAL: wrap resulting header with your guard.
# (c) toiletbril

# strip old guards and pragmas.
remove_header_guards = True

# resulting header will be wrapped with this definition.
# for no guard, leave this blank.
header_guard = "HEADER_H"


def merge_hpp_files(directory_path):
    hpp_files = glob(path.join(directory_path, "**/*.hpp"), recursive=True)
    hpp_files += glob(path.join(directory_path, "**/*.h"), recursive=True)

    includes = set()

    # move non-local includes to the top
    for file_path in hpp_files:
        with open(file_path, "r") as file:
            for line in file:
                if line.startswith("#include \""):
                    continue
                if line.startswith("#include <"):
                    includes.add(line)

    file_contents = []

    for file_path in hpp_files:
        with open(file_path, "r") as file:

            content = file.readlines()
            content = [
                line for line in content if not line.startswith("#include")
            ]

            pattern = compile(r"^#(define|ifndef) *.*_H_{0,}")
            scope = -1
            cleaned_content = []

            # int main(void) { return 0; }

            # remove patterns:
            # '#pragma once'
            # '#ifndef <macro>_H'
            # '#define <macro>_H'
            if remove_header_guards:
                for line in content:
                    if pattern.match(line):
                        scope = 0
                    elif line.startswith("#if"):
                        scope += 1
                        cleaned_content.append(line)
                    elif line.startswith("#endif"):
                        if scope == 0:
                            continue
                        else:
                            scope -= 1
                            cleaned_content.append(line)
                    elif line.startswith("#pragma once"):
                        continue
                    else:
                        cleaned_content.append(line)
            else:
                cleaned_content = content

            file_contents.append((file_path, cleaned_content))

    non_local_includes = sorted(list([line.strip() for line in includes]))

    merged_content: list[str] = []

    merged_content.append(
        "/**\n * This header was automatically generated.\n *\n * Following files were merged:\n "
        + ",\n * ".join(hpp_files)
        + "\n */\n")

    if header_guard:
        merged_content.append(
            f"\n#ifndef {header_guard}\n#define {header_guard}\n")
    merged_content.append("\n")
    merged_content.append("\n".join(non_local_includes) + "\n\n")

    for file_path, content in file_contents:
        merged_content.append(
            f"/*\n * Filename: {path.basename(file_path)}\n */\n")
        merged_content.extend(content)
        merged_content.append("\n")

    if header_guard:
        merged_content.append(f"#endif // {header_guard}\n")

    # remove two and more consecutive blank lines and print to stdout
    blank = False
    for line in merged_content:
        if line == "\n":
            if blank:
                continue
            blank = True
        else:
            blank = False

        print(line, end="")


if __name__ == "__main__":
    if len(argv) != 2:
        print("USAGE: python3 combine_headers.py <directory>")
        exit(1)

    directory_path = argv[1]
    merge_hpp_files(directory_path)
