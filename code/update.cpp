//
//  Update a specified RID.
//  update.cpp
//  CSC443-A2
//
//  Created by Daniel Ortiz Costa on 2017-11-17.
//  Copyright © 2017 Daniel Ortiz Costa. All rights reserved.
//

#include <stdio.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include <cstring>
#include "library.hpp"


int main(int argc, char *argv[])
{
    if (argc != 6)
    {
        throw std::runtime_error("Required: <heapfile> <record_id> <attribute_id> <new_value> <page_size>.");
    }
    
    // VALIDATE INPUTS.
    
    char* heap_file_path = argv[1];
    std::string record_id(argv[2]);
    int new_attr_id = std::atoi(argv[3]);
    char* new_val = argv[4];
    int page_size = std::atoi(argv[5]);
    
    if(page_size <= 0) throw std::runtime_error("Invalid page size.");
    if(new_attr_id < 0 || new_attr_id > NUMBER_OF_ATTRIBUTES || std::strlen(new_val) != ATTRIBUTE_SIZE)
    {
        throw std::runtime_error("Invalid attribute information.");
    }
    
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
    
    
    // LOAD HEAP FILE AND UPDATE.
    
    Heapfile heapfile;
    heapfile.page_size = page_size;
    heapfile.file_ptr = fopen(heap_file_path, "r+");
    if(!heapfile.file_ptr) throw std::runtime_error("Could not open heapfile!");
    
    bool updated = false;
    RecordIterator record_iter(&heapfile);
    while(record_iter.hasNext())
    {
        int last_page_id = record_iter.getLastPageIDRead();
        int last_slot_read = record_iter.getLastSlotRead();
        Record r = record_iter.next();
        
        if(last_page_id > update_page) break;
        if(record_iter.isLastSlotEmpty()) continue;
        if(last_page_id != update_page || last_slot_read != update_slot) continue;
        
        V prev = r[new_attr_id];
        r[new_attr_id] = new_val;
        
        Page p = record_iter.getLastPageRead();
        write_fixed_len_page(&p, update_slot, &r);
        write_page(&p, &heapfile, update_page);
        
        printf("Updated RID(%d.%d)[%d]: '%s' -> '%s' \n",
               last_page_id, last_slot_read, new_attr_id, prev, new_val);
        
        updated = true;
        break;
    }
    
    if(!updated) printf("Could not find page / record.\n");
    fclose(heapfile.file_ptr);
    
    return 0;
}
