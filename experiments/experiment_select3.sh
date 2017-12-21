#!/bin/bash

declare -a page_sizes=(1000 1024 2000 3000 4000 4096 8000 12000 20000 40000 100000 150000 400000)

project_path="/Users/daniel/Desktop/A2"

executable="$project_path/executables/select3"
storage_path="$project_path/storage/colstore"

printf "SELECT3 TESTS\n"
for i in "${page_sizes[@]}"
do
	colstorebase="$storage_path/colstore$i"
	time $executable $colstorebase 4  9 ZZAA ZZZZ $i | grep -E '(SELECT|Total|Error)'
	printf "\n"
done