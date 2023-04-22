#!/bin/bash

GREEN='\033[0;32m'
NC='\033[0m' # No Color


# Change directory to the directory containing the Makefiles
cd ./one-one
printf "${GREEN}Executing One - One ..!!${NC}\n\n"
make
make clean
printf "\n\n${GREEN}Completed Execution Of One - One ..!!${NC}\n\n"

cd ../many-one

printf "${GREEN}Executing Many - One ..!!${NC}\n\n"
make
make clean
printf "\n\n${GREEN}Completed Execution Of Many - One ..!!${NC}\n\n"

cd ../many-many

printf "${GREEN}Executing Many - Many ..!!${NC}\n\n"
make
make clean
printf "\n\n${GREEN}Completed Execution Of Many - Many ..!!${NC}\n\n"
