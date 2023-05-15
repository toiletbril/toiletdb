#!/usr/bin/env python3

from sys import argv

from random import choice, randint

name_one = ["Kamil", "Olivia", "Arnold", "Nicholas", "Alex", "Xiao", "Carl", "Ignat", "Jensen", "Emma", "Tyler", "Ash", "Jeff", "Bob", "Pip", "Maryk", "Oleg", "Ivan", "John", "Mahmoud", "Maria", "Max", "Joe", "Ronald"]

name_two = ["berg", "onson", "onen", "ski", "unts", "fitz", "lu", "war", "zu", "eanu", "ent", "idis", "kan", "jan", "lang", "wolf", "gang", "min", "tz"]

groups = [
    "Testers",
    "Humans",
    "Americans",
    "Europeans",
    "Boylikers",
    "Vegeterians"
]

def _generate_name() -> str:
    return choice(name_one)

def _generate_surname() -> str:
    return choice(name_one) + choice(name_two)

def generate_entry(i: int) -> str:
    return (f"|{i}"
            f"|{_generate_name()}"
            f"|{_generate_surname()}"
            f"|{choice(groups)}"
            f"|{randint(100000, 999999)}"
             "|\n")

def main():

    if (len(argv) != 3):
        print("Usage: makedb <count> <filename>")
        exit(1)

    count = int(argv[1])
    filename = argv[2]

    with open(filename, "w") as f:
        for i in range(count):
            f.write(generate_entry(i))

    print("Done.")

if __name__ == "__main__":
    main()
