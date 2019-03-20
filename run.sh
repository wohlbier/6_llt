#!/bin/bash

if [ "$#" -ne 1 ]; then
    echo "Usage: ./run.sh program"
    exit 1
fi

PROG=$1
OUTPUT=$1.txt

echo "Program: $PROG"
echo "Output: $OUTPUT"

rm $OUTPUT

make clean
make run
cat $PROG.cdc >> $OUTPUT

cat $OUTPUT
