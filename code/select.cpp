//
//  select.cpp
//  CSC443-A2
//
//  Created by Daniel Ortiz Costa on 2017-11-17.
//  Copyright © 2017 Daniel Ortiz Costa. All rights reserved.
//

#include <stdio.h>
#include <string>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include "library.hpp"

int main(int argc, char *argv[])
{
    const int FROM = 1;
    const int TO = 5;
    
    if (argc != 6)
    {
        throw std::runtime_error("Required: <heap_file_name> <attribute_id> <start> <end> <page_size>.");
    }
    
    char* heap_file_path = argv[1];
    int page_size = std::atoi(argv[5]);
    if(page_size <= 0) throw std::runtime_error("Invalid page size.");
    
    int attr_id = std::atoi(argv[2]);
    if(attr_id < 0 || attr_id > NUMBER_OF_ATTRIBUTES) throw std::runtime_error("Invalid attribute ID.");
    
    std::string START(argv[3]);
    std::string END(argv[4]);
    
    if(START.compare(END) > 0) throw std::runtime_error("START > END!");
    
    
    // LOAD HEAPFILE AND SELECT.
    
    Heapfile heapfile;
    heapfile.page_size = page_size;
    heapfile.file_ptr = fopen(heap_file_path, "r");
    if(!heapfile.file_ptr) throw std::runtime_error("Could not open heapfile!");
    
    RecordIterator record_iter(&heapfile);
    size_t selected = 0;
    size_t total = 0;
    while(record_iter.hasNext())
    {
        int last_page_id = record_iter.getLastPageIDRead();
        int last_slot_read = record_iter.getLastSlotRead();
        Record r = record_iter.next();
        total += 1;
        
        if(record_iter.isLastSlotEmpty()) continue;
        
        char* attribute = new char[ATTRIBUTE_SIZE + 1]();
        memcpy(attribute, r[attr_id], ATTRIBUTE_SIZE);
        std::string attribute_str(attribute);
        delete[] attribute;
        
        if(START.compare(attribute_str) > 0 ||  END.compare(attribute_str) < 0) continue;
        
        attribute[TO + 1] = '\0';
        selected += 1;
        printf("Record RID(%d.%d)[%d][%d:%d] '%s'\n",
               last_page_id, last_slot_read, attr_id, FROM, TO, attribute + FROM);

    }
    printf("Total tuples = %lu/%lu.\n", selected, total);
    fclose(heapfile.file_ptr);
    return 0;
}
