#!/bin/bash

# Change directory to the directory containing the Makefiles
cd ./one-one
printf "Executing One - One ..!!\n\n"
make
make clean
printf "\n\nCompleted Execution Of One - One ..!!"

cd ../many-one

printf "Executing Many - One ..!!\n\n"
make clean
printf "\n\nCompleted Execution Of Many - One ..!!"

cd ../many-many

printf "Executing Many - Many ..!!\n\n"
make clean
printf "\n\nCompleted Execution Of Many - Many ..!!"
