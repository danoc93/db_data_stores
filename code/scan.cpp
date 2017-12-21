//
//  Scan all records in a heap file and print them out.
//  scan.cpp
//  CSC443-A2
//
//  Created by Daniel Ortiz Costa on 2017-11-16.
//  Copyright © 2017 Daniel Ortiz Costa. All rights reserved.
//

#include <stdio.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include "library.hpp"

int main(int argc, char *argv[])
{
    if (argc != 3) throw std::runtime_error("Required: <heap_file_name>, <page_size>.");
    
    char* heap_file_path = argv[1];
    int page_size = std::atoi(argv[2]);
    if(page_size <= 0) throw std::runtime_error("Invalid page size.");
    
    Heapfile heapfile;
    heapfile.page_size = page_size;
    heapfile.file_ptr = fopen(heap_file_path, "r");
    if(!heapfile.file_ptr) throw std::runtime_error("Could not open heapfile!");
    
    RecordIterator record_iter(&heapfile);
    while(record_iter.hasNext())
    {
        int last_page_id = record_iter.getLastPageIDRead();
        int last_slot_read = record_iter.getLastSlotRead();
        Record r = record_iter.next();
        
        if(record_iter.isLastSlotEmpty()) continue;
        
        printf("Record RID(%d.%d)\n", last_page_id, last_slot_read);
        print_record(r);
        printf("\n");
    }
    fclose(heapfile.file_ptr);
    return 0;
}
