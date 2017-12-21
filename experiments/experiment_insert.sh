#!/bin/bash

declare -a page_sizes=(1000 1024 2000 3000 4000 4096 8000 12000 20000 40000 100000 150000 400000)

project_path="/Users/daniel/Desktop/A2"

executable="$project_path/executables/insert"
test_dataset_full_path="$project_path/data/test500.csv"
storage_path="$project_path/storage/heapstorage"

printf "INSERT TESTS\n"
for i in "${page_sizes[@]}"
do
	printf "PROCESSED:"
	time $executable $storage_path/heapfile$i $test_dataset_full_path $i | grep -c RID
	printf "\n"
done