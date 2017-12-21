//
//  Delete a specified record!
//  delete.cpp
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
    if (argc != 4)
    {
        throw std::runtime_error("Required: <heapfile> <record_id> <page_size>.");
    }
    
    // VALIDATE INPUTS.
    
    char* heap_file_path = argv[1];
    std::string record_id(argv[2]);
    int page_size = std::atoi(argv[3]);
    
    if(page_size <= 0) throw std::runtime_error("Invalid page size.");
    
    size_t token = record_id.find(".");
    std::string page = record_id.substr(0, token);
    std::string slot = record_id.substr(token + 1, record_id.length());
    
    PageID update_page = atoi(page.c_str());
    PageID update_slot = atoi(slot.c_str());
    
    if(update_page >= NUM_DIRECTORY_ENTRIES ||
       update_slot >= page_size / (NUMBER_OF_ATTRIBUTES * ATTRIBUTE_SIZE))
    {
        throw std::runtime_error("Invalid RID range!");
    }
    
    
    // LOAD HEAP FILE AND DELETE.
    
    Heapfile heapfile;
    heapfile.page_size = page_size;
    heapfile.file_ptr = fopen(heap_file_path, "r+");
    if(!heapfile.file_ptr) throw std::runtime_error("Could not open heapfile!");
    
    bool deleted = false;
    RecordIterator record_iter(&heapfile);
    while(record_iter.hasNext())
    {
        int last_page_id = record_iter.getLastPageIDRead();
        int last_slot_read = record_iter.getLastSlotRead();
        Record r = record_iter.next();
        
        if(last_page_id > update_page) break;
        if(record_iter.isLastSlotEmpty()) continue;
        if(last_page_id != update_page || last_slot_read != update_slot) continue;
        
        Page p = record_iter.getLastPageRead();
        V empty = new char[ATTRIBUTE_SIZE]();
        for(int i = 0; i < NUMBER_OF_ATTRIBUTES; i++) r[i] = empty;
        write_fixed_len_page(&p, update_slot, &r);
        write_page(&p, &heapfile, update_page);
        delete[] empty;
        printf("Deleted RID (%d.%d).\n", last_page_id, last_slot_read);
        deleted = true;
        break;
    }
    
    if(!deleted) printf("Could not find page / record.\n");
    fclose(heapfile.file_ptr);
    
    return 0;
}

