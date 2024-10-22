#!/bin/bash
umask 077
# Check if the correct number of arguments is provided
if [ $# -ne 2 ]; then
  echo "Error: Usage: Two arguments required: <directory> <search string>"
  exit 1
fi

# Assigning input arguments to variables
filesdir=$1
searchstr=$2

# Check if the first argument is a valid directory
if [ ! -d "$filesdir" ]; then
  echo "Error: $filesdir is not a directory"
  exit 1
fi

# Find the number of files (recursively) in the directory
num_files=$(find "$filesdir" -type f | wc -l)

# Find the number of lines that match the search string
num_matching_lines=$(grep -r "$searchstr" "$filesdir" 2>/dev/null | wc -l)

# Print the result
echo "The number of files are $num_files and the number of matching lines are $num_matching_lines"

exit 0
