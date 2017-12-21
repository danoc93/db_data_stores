#!/bin/bash

declare -a page_sizes=(1000 1024 2000 3000 4000 4096 8000 12000 20000 40000 100000 150000 400000)

project_path="/Users/daniel/Desktop/A2"

executable="$project_path/executables/write_fixed_len_pages"
test_dataset_full_path="$project_path/data/test2000.csv"
storage_path="$project_path/storage/pagefiles"

printf "\nWRITE TESTS\n"
for i in "${page_sizes[@]}"
do
	$executable $test_dataset_full_path $storage_path/pagefile$i $i
done