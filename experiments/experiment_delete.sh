#!/bin/bash

declare -a page_sizes=(1000 1024 2000 3000 4000 4096 8000 12000 20000 40000 100000 150000 400000)

project_path="/Users/daniel/Desktop/A2"

executable="$project_path/executables/delete"
storage_path="$project_path/storage/heapstorage"

printf "DELETE TESTS\n"
for i in "${page_sizes[@]}"
do
	time $executable $storage_path/heapfile$i 4.0 $i 
	printf "\n"
done