#!/bin/bash

declare -a page_sizes=(1000 1024 2000 3000 4000 4096 8000 12000 20000 40000 100000 150000 400000)

project_path="/Users/daniel/Desktop/A2"

executable="$project_path/executables/csv2colstore"
storage_path="$project_path/storage/colstore"
test_dataset_full_path="$project_path/data/test2000.csv"

printf "COL STORE BUILDING TESTS\n"
for i in "${page_sizes[@]}"
do
	colstorebase="$storage_path/colstore$i"
	mkdir -p foo $colstorebase
	time $executable $test_dataset_full_path $colstorebase $i
	printf "\n"
done