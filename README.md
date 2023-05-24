# toiletdb
In-memory database with an ability to serialize into readable files and a CLI for it.

Database files look like this:
```
tdb<format version>
|[modifier] <column type> <column name>| ...
|<value of column type>| ...
```

Available types are:

- `int` 32 bit signed integer
- `b_int` 64 bit unsigned integer
- `str` Array of 8 bit characters

Modifiers:

- `const` Constant
- `id` Column that will be used for indexing (only of type `b_int`)

## Why

I got this idea from my friends' course project.
The actual task was too easy, so I decided to implement a whole database instead :3c.

## Usage

```console
$ toiletdb <database file>
```

```console
test# help
Available commands:
        help    ?               See this message.
        exit    q, quit         Save and quit. Append '!' to the end to skip saving.
        search  s               Search the database.
        list    ls              List all students.
        size                    See total amount of students in database.
        add                     Add a student to database.
        remove  rm              Remove a student from database.
        edit    e               Edit student's details.
        clear                   Clear the database.
        commit  save            Save changes to the file.
        revert  reverse         Revert uncommited changes.
```

For testing purposes, you can generate database file with:
```console
$ python3 testing/makedb.py <count of entries> <filename> [name complexity]
```

## Building

`./build.sh` (POSIX), `./build.bat` (Windows Clang). Look for binary in `bin/`
