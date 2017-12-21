//
//  Create a Heap File from the CSV.
//  csv2heapfile.cpp
//  CSC443-A2
//
//  Created by Daniel Ortiz Costa on 2017-11-15.
//  Copyright © 2017 Daniel Ortiz Costa. All rights reserved.
//

#include "library.hpp"
#include <stdio.h>
#include <string>
#include <iostream>
#include <math.h>
#include <cstdlib>

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Error! Required: <src_csv_file>, <heap_file_name>, <page_size>.\n");
        return -1;
    }
    
    std::string csv_file_name(argv[1]);
    std::string heap_file_path(argv[2]);
    int page_size = std::atoi(argv[3]);
    
    size_t num_of_attributes;
    char* attr_data = get_attribute_data(csv_file_name, &num_of_attributes);
    
    size_t num_records = num_of_attributes / NUMBER_OF_ATTRIBUTES;
    if(num_of_attributes % NUMBER_OF_ATTRIBUTES)
    {
        printf("Error: Inconsistent attribute / record size ratio.\n");
        delete[] attr_data;
        return -1;
    }
    
    int record_size;
    std::vector<Record> records = extract_record_vector(attr_data, num_records, &record_size);
    delete[] attr_data;
    
    if(page_size < record_size)
    {
        printf("Error: Cannot write data to pages. Page Size (%d) < Record Size (%d).\n",
               page_size, record_size);
        clear_records(records);
        return -1;
    }
    
    printf("Building heap file %s\n", heap_file_path.c_str());
    printf("Heap Dir Entries: %d, Dir Size: %d bytes, Page Size: %d bytes\n",
           NUM_DIRECTORY_ENTRIES, DIRECTORY_PAGE_SIZE, page_size);
    
    FILE *heap_file_ptr;
    heap_file_ptr = fopen(heap_file_path.c_str(), "w+");
    
    if(!heap_file_ptr) throw std::runtime_error("Could not open heap file!");
    
    Heapfile heap_file;
    init_heapfile(&heap_file, page_size, heap_file_ptr);
    
    size_t pages_needed = ceil((record_size * records.size()) / (float)page_size);
    if(pages_needed > NUM_DIRECTORY_ENTRIES || pages_needed < 1)
    {
        printf("Error: Need %lu pages to serialize records! ", pages_needed);
        printf("Adjust NUM_DIRECTORY_ENTRIES.\n");
        clear_records(records);
        fclose(heap_file_ptr);
        return -1;
    }
    
    printf("Serialization (%lu PAGES x %d bytes), (%lu RECORDS x %d bytes)\n",
           pages_needed, page_size, num_records, ATTRIBUTE_SIZE*NUMBER_OF_ATTRIBUTES);
    
    std::vector<Page> pages = paginate_records(records, page_size, record_size);

    // Allocate first, otherwise checksums will be deceiving.
    for(size_t i = 0; i < NUM_DIRECTORY_ENTRIES; i++) alloc_page(&heap_file);
    for(size_t i = 0; i < pages.size(); i++)
    {
        write_page(&pages[i], &heap_file, (PageID)i);
        free(pages[i].data);
    }
    
    pages.clear();
    clear_records(records);
    fclose(heap_file_ptr);
    
    return 0;
}
