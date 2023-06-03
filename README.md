# toiletdb

**THIS SOFTWARE IS UNFINISHED, UNSAFE, AND IS NOT SUITABLE FOR PRODUCTION. USE IT AT YOUR OWN RISK.**

In-memory database with an ability to serialize into readable files.

I got this idea from my friends' course project.
The actual task was too easy, so I decided to implement a whole database :3c.

## Table of contents

- [Introduction](#introduction)
    - [Library](#library)
    - [File format](#file-format)
- [Building](#building-from-source)
- [CLI](#cli)
    - [Usage](#usage)

## Introduction

### Library

> TODO, see [`include/toiletdb.hpp`](include/toiletdb.hpp)

### File format

Database files look like this:

```
tdb<format version>
|[modifier] <column type> <column name>| ...
|<value of column type>| ...
|<another value of column type>| ...
...
```

Available column types are:

- `int`   Signed integer
- `uint`  Unsigned integer
- `str`   Character string

Modifiers:

- `const` Marks column as not editable through code (you can still edit it manually :3)
- `id`    Marks column to be used for indexing (only for `const uint`)

## Building from source

#### Windows

Static library and CLI (launch this from MSVC-enabled console):
```console
> ./build_msvc.bat
```

Alternatively, you can use `make` targets from below with:

- `g++`/`clang++` (CXX variable in [`Makefile`](./Makefile))
- `make`, `ar` (You can get those with MinGW32)
    - `mingw32-make.exe`
    - `x86_64-w64-mingw32-ar.exe`

#### POSIX

Static library:

```console
$ make release
```

Copy include header from [`include/toiletdb.hpp`](include/toiletdb.hpp), static library from `build/toiletdb.lib` and do whatever.

CLI:

```console
$ make cli
```

Look for binaries in `build/`.

## CLI

### Usage

```console
$ toiletdb <database file>
```

```console
test# help
Available commands:
        help    ?               See this message.
        version v, ver          Display version.
        exit    q, quit         Save and quit. Append '!' to the end to skip saving.
        search  s               Search the database.
        list    ls              Show all rows.
        types   lst             Show only a table header.
        size                    See total amount of rows in database.
        add                     Add a row to database.
        remove  rm              Remove a row from database.
        edit    e               Edit a row.
        clear                   Clear the database.
        commit  save            Save changes to the file.
        revert  reverse         Revert uncommited changes.
```

For testing purposes, you can generate mock student database file with:

```console
$ python3 scripts/make_db.py <filename> [row count] [name complexity, number from 1 to 5+]
```
