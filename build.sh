ENTRY=main.cpp
FLAGS="-O2 -Wall -Wextra -pedantic"
SRC=src
OUT=bin
OUTFILE=toiletdb

mkdir -p $OUT

set -x

c++ $FLAGS $SRC/$ENTRY -o $OUT/$OUTFILE
