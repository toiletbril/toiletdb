# toiletdb

**THIS SOFTWARE IS UNFINISHED, UNSAFE, AND IS NOT SUITABLE FOR PRODUCTION. USE IT AT YOUR OWN RISK.**

In-memory database with an ability to serialize into readable files and a CLI for it.

I got this idea from my friends' course project.
The actual task was too easy, so I decided to implement a whole database instead :3c.

## Table of contents

- [Introduction](#introduction)
	- [Library](#library)
	- [File format](#lile-format)
- [Building](#building)
	- [POSIX](#posix)
	- [Windows](#windows)
- [CLI](#cli)
	- [Usage](#usage)

## Introduction

### Library

> TODO (see [`include/toiletdb.hpp`](include/toiletdb.hpp))

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

- `int`   32 bit signed integer
- `b_int` 64 bit unsigned integer
- `str`   Array of 8 bit characters

Modifiers:

- `const` Marks column as not editable through code (you can still edit it manually :3)
- `id`    Marks column to be used for indexing (only for `b_int`)

## Building from source

### Prerequisites
#### POSIX

- GCC/Clang
- `make`
- `ar`

#### Windows

- GCC/Clang
- MinGW32
	- `mingw32-make.exe`
	- `x86_64-w64-mingw32-ar.exe`

### Build

`build/toiletdb.lib` (static library)
```console
make release
```

Copy include header from [`include/toiletdb.hpp`](include/toiletdb.hpp), static library from `build/toiletdb.lib` and do whatever.
When building your project, put `toiletdb.lib` along your source file names.

`build/toiletdb` (CLI)
```console
make cli
```

## CLI

### Usage

```console
$ toiletdb <database file>
```

```console
test# help
Available commands:
	help  	?		See this message.
	exit  	q, quit		Save and quit. Append '!' to the end to skip saving.
	search	s		Search the database.
	list  	ls		Show all rows.
	types 	lst		Show only a table header.
	size  			See total amount of rows in database.
	add   			Add a row to database.
	remove	rm		Remove a row from database.
	edit  	e		Edit a row.
	clear 			Clear the database.
	commit	save		Save changes to the file.
	revert	reverse		Revert uncommited changes.
```

For testing purposes, you can generate student database file with:
```console
$ python3 scripts/make_db.py <count of entries> <filename> [name complexity, number from 1 to 5]
```