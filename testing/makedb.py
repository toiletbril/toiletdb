#!/usr/bin/env python3

from sys import argv

import data

if (len(argv) != 3):
    print("Usage: makedb <count> <filename>")
    exit(1)

count = int(argv[1])
filename = argv[2]

with open(filename, "w") as f:
    for i in range(count):
        f.write(data.generate_entry(i))

print("Done.")
