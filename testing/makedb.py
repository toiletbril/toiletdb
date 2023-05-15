#!/usr/bin/env python3

from sys import argv
from random import choice, randint

import data

if (len(argv) != 3):
    print("Usage: makedb <count> <filename>")
    exit(1)

count = int(argv[1])
filename = argv[2]

with open(filename, "w") as f:
    for i in range(count):
        f.write(
            f"|{i}|{choice(data.names)}"
            f"|{choice(data.surnames)}"
            f"|{choice(data.groups)}"
            f"|{randint(100000, 999999)}|\n")

print("Done.")
