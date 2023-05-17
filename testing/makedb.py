#!/usr/bin/env python3

from sys import argv

from random import choice, randint

name_one = ["Kamil", "Olivia", "Arnold", "Nicholas", "Alex", "Xiao", "Carl", "Ignat", "Jensen", "Emma", "Tyler", "Ash", "Jeff", "Bob", "Pip", "Maryk", "Oleg", "Ivan", "John", "Mahmoud", "Maria", "Max", "Joe", "Ronald"]

name_two = ["berg", "onson", "onen", "ski", "unts", "fitz", "lu", "war", "zu", "eanu", "ent", "idis", "kan", "jan", "lang", "wolf", "gang", "min", "tz"]

groups = [
    "Testers",
    "Windows",
    "Vacuum cleaners",
    "Gamers",
    "Boylikers",
    "Vegeterians",
    "Pilers"
]

def _generate_name(suffixes: int) -> str:
    return choice(name_one) + "".join([ choice(name_two) for _ in range(suffixes) ])

def _generate_surname(suffixes: int) -> str:
    return choice(name_one) + "".join([ choice(name_two) for _ in range(suffixes) ])

def _generate_groups(suffixes: int) -> str:
    return choice(groups) + ((", " + ", ".join([ choice(groups) for _ in range(suffixes) ])) if suffixes else "")

def generate_entry(i: int, suffixes: int) -> str:
    return (f"|{i}"
            f"|{_generate_name(suffixes)}"
            f"|{_generate_surname(suffixes)}"
            f"|{_generate_groups(0)}"
            f"|{randint(10000000, 99999999)}"
             "|\n")

def main():
    if (len(argv) < 3):
        print("Usage: makedb <count> <filename> [name complexity]")
        exit(1)

    count = int(argv[1])
    filename = argv[2]
    suffixes = 3

    if (len(argv) > 3):
        suffixes = int(argv[3])

    with open(filename, "w") as f:
        for i in range(count):
            f.write(generate_entry(i, suffixes))

    print("Done.")

if __name__ == "__main__":
    main()
