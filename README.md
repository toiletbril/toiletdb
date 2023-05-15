# toiletdb

In-memory database with an ability to serialize into a file and a CLI for it.

Implemented as a database of students. For now, file format looks like this:
```
|<ID>|<name>|<surname>|<group name>|<record book number>|
```

## Why

I got this idea from my friends' course project.
Of course, the actual task was too easy, so I decided to implement a whole database instead :3c.

## Usage

```console
$ toiletdb <database file>
```

## Bulding

`./build.sh` (POSIX), `./build.bat` (Windows). Look for binary in `bin/`
