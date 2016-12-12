#!/bin/bash

show_usage () {
  echo "usage: $(basename "$0") [-h|--help]"
}

show_help () {
  show_usage
  echo "This scipt read args line by line, performing the next transformation:
1. keep first as is, like it is command.
2. escape double quotes in all lines
3. quote every line by double quotes
4. concat all lines using '\0' symbol

Script works with streams, files and stdin.

Options:
  -h, --help          shows this help
"
}

# Do it simple
if [ "$1" == "-h" ] || [ "$1" == "--help" ]; then
  show_help
  exit
fi

read command
params="$(sed 's_\([\\"]\)_\\\1_g' | sed 's/\(.*\)/"\1"/g' | awk 'BEGIN{FS="\n"; ORS="\0";}{print}')"
#params="$(sed 's_\("\)_\\\1_g' | sed 's/\(.*\)/"\1"/g' | awk '{printf "%s%s", sep,$1; sep="|"}' FS="\n")"
echo "$command $params"
