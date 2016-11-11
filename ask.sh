#!/bin/bash

show_usage () {                        
  echo "usage: $(basename "$0") [-h|--help] <QUESTION> [<DEFAULT_ANSWER>]"
}

show_help () {
  show_usage
  echo "Asks question untill positive or negative answer.
Correct answer is string that starts with one of letters: [y, n, Y, N].
In case of answer is not specified and <DEFAULT_ANSWER> is provided - it will be used as answer.

Positional args:
  QUESTION            question that will be asked to user untill his answer
  DEFAULT_ANSWER      [Y|N] - answer, that will be used in case of empty string

Options:
  -h, --help          shows this help

Exit status:
  0: positive answer
  1: negative answer
"
}

if [ "$#" == "0" ]; then
  show_usage
  exit
fi

# Do it simple
if [ "$1" == "-h" ] || [ "$1" == "--help" ]; then 
  show_help
  exit
fi

# Check default value
if [ -n "$2" ] && [ "$2" != "Y" ] && [ "$2" != "--help" ]; then 
  show_help
  exit
fi

[[ $2 = "Y" ]] && choice="[Y/n]" || [[ $2 = "N" ]] && choice="[Y/n]" || choice="[y/n]"
a="-"
while ([ "$a" != "Y" ] && [ "$a" != "N" ]) && ([ -n "$a" ] || [ ! "$2" ])
do
  echo "$1" "$choice"
  IFS="\n"
  read a
  a=${a:0:1}
  a=${a^}
done

# Set default value if empty string
[[ "$a" == "" ]] && a="$2"

if [ "$a" == "Y" ]; then
  exit 0
else
  exit 1
fi
