#!/bin/sh

if [ $# -lt 2 ]
then
    echo "Missing argument"
    exit 1
elif [ $# -gt 2 ]
then
    echo "Too many argument"
    exit 1
else
    DIR=$1
    SEARCH=$2
fi

if [ ! -d $DIR ]
then
    echo "Dir path ${DIR} is not a valid folder"
    exit 1
fi

grep -rc $SEARCH $DIR | awk -F: '
$NF > 0 {
    files++;
    lines += $NF
}
END {
    print "The number of files are " (files+0) " and the number of matching lines are " (lines+0)
}'