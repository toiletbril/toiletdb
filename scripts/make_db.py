#!/usr/bin/env python3

from sys import argv

from random import choice, randint

name_one = ["Kamil", "Olivia", "Arnold", "Nicholas", "Alex", "Xiao", "Carl", "Ignat", "Jensen", "Emma", "Tyler", "Ash", "Jeff", "Bob", "Pip", "Maryk", "Oleg", "Ivan", "John", "Mahmoud", "Maria", "Max", "Joe", "Ronald"]
name_two = ["berg", "onson", "onen", "ski", "unts", "fitz", "lu", "war", "zu", "eanu", "ent", "idis", "kan", "jan", "lang", "wolf", "gang", "min", "tz"]
groups   = ["Testers", "Windows", "Vacuum cleaners", "Gamers", "Boylikers", "Vegeterians", "Pilers", "Dudes", "Army", "Band", "Batch", "Gang", "Alliance"]

def name(suffixes: int) -> str:
    return choice(name_one) + "".join([ choice(name_two) for _ in range(suffixes) ])

def surname(suffixes: int) -> str:
    return choice(name_one) + "".join([ choice(name_two) for _ in range(suffixes) ])

def group(suffixes: int) -> str:
    return choice(groups) + ((", " + ", ".join([ choice(groups) for _ in range(suffixes) ])) if suffixes else "")

def generate_entry(suffixes: int) -> str:
    return (f"|{randint(0, 99999999)}|{name(suffixes)}|{surname(suffixes)}|{group(0)}|{randint(10000000)}|\n")

def main():
    if (len(argv) < 3):
        print("USAGE: python3 make_db.py <filename> [row count] [name complexity, number from 1 to 5+]")
        exit(1)

    filename = argv[1]
    count    = 1000
    suffixes = 3

    if (len(argv) > 2):
        count = int(argv[2])

    if (len(argv) > 3):
        suffixes = int(argv[3])

    with open(filename, "w") as f:
        f.write("tdb1\n|id b_int ID|str Name|str Surname|str Group|b_int Number|\n")
        for _ in range(count):
            f.write(generate_entry(suffixes))

    print("Done.")

if __name__ == "__main__":
    main()
