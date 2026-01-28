#!/bin/bash
read -p "Enter a directory: " DIR
echo "Counting file types in: $DIR"
echo "TXT files: $(find "$DIR" -type f -name "*.txt" | wc -l)"
echo "SH files: $(find "$DIR" -type f -name "*.sh" | wc -l)"
echo "C files: $(find "$DIR" -type f -name "*.c" | wc -l)"
