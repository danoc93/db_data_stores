#!/bin/bash

declare -a page_sizes=(1000 1024 2000 3000 4000 4096 8000 12000 20000 40000 100000 150000 400000)

project_path="/Users/daniel/Desktop/A2"

executable="$project_path/executables/read_fixed_len_page"
storage_path="$project_path/storage/pagefiles"

printf "\nREAD TESTS\n"
for i in "${page_sizes[@]}"
do
	$executable $storage_path/pagefile$i $i
done