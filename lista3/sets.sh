#!/bin/bash

function help {
    echo "Usage: {-u | -i | -r} FILE1 FILE2
    -u: union of FILE1 and FILE2
    -i: intersection of FILE1 and FILE2
    -r: substract FILE2 from FILE1"
}

if [ $# -ne 3 ]
#if [ \( $# -ne 3 \) -o ! \( $1 = "-u" -o $1 = "-i" \)  ] 
then
    help
    exit
fi

TMP1=$(mktemp)
TMP2=$(mktemp)

echo "File 1:" "$2"
echo "File 2:" "$3"
sort "$2" > $TMP1
sort "$3" > $TMP2

case $1 in
    "-u")
        OUT=$(comm "$TMP1" "$TMP2")
        echo -e "Mode: union\n---------------"
        ;;
    "-i")
        OUT=$(comm -12 "$TMP1" "$TMP2")
        echo -e "Mode: intersect\n---------------"
        ;;
    "-r")
        OUT=$(comm -1 "$TMP1" "$TMP2")
        echo -e "Mode: substract\n---------------"
        ;;
    *) 
        help
        exit
        ;;
esac

for l in $OUT; do echo $l; done

rm $TMP1
rm $TMP2
