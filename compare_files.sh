#!/bin/bash

files=("haha.txt" "large.txt")
exit_code=0

echo "Comparison results:"

for file in "${files[@]}"
do
  if cmp -s "t0/$file" "t1/$file"; then
    printf "  t0/%-10s equal to t1/%s\n" "$file" "$file"
  else
    echo "Error: $file is not equal"
    exit_code=1
  fi
done

if [ $exit_code -eq 0 ]; then
  echo "All files are equal"
fi

exit $exit_code

