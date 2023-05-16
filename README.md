# toiletdb

Persistent, in-memory database with an ability to serialize and a CLI for it.

Implemented as a table of students. Database files look like this:
```
|<ID>|<name>|<surname>|<group name>|<record book number>|
```

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
        help            See this message.
        exit            Save and quit. Append '!' to the end to skip saving.
        search          Search the database.
        id              Search by ID.
        list            List all students.
        size            See total amount of students in database.
        add             Add a student to database.
        remove          Remove a student from database.
        edit            Edit student's details.
        grades          See student's grades.
        clear           Clear the database.
        commit          Save changes to the file.
        revert          Revert uncommited changes.
```

For testing purposes, you can generate database file with:
```console
$ python3 testing/makedb.py <count of entries> <filename>
```

## Building

`./build.sh` (POSIX), `./build.bat` (Windows Clang). Look for binary in `bin/`
