#!/bin/bash

./compare_files.sh

if [ $? -eq 0 ]; then
  rmh
else
  exit 1
fi

