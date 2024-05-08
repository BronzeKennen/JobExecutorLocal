#!/bin/bash

if [ $# -eq 0 ]; then
    echo "Usage: $0 <file_with_commands>"
    exit 1
fi

for file in "$@"; do
    if [ ! -f "$1" ]; then
    	echo "File $1 not found."
    	exit 1
    fi

    while IFS= read -r line; do
        ./jobCommander issueJob "$line"
    done < "$file"
done

