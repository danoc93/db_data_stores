//
//  Insert all the records from a csv file into a heap file.
//  insert.cpp
//  CSC443-A2
//
//  Created by Daniel Ortiz Costa on 2017-11-17.
//  Copyright © 2017 Daniel Ortiz Costa. All rights reserved.
//

#include <stdio.h>
#include <string>
#include <iostream>
#include <cstdlib>
#include "library.hpp"


int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        throw std::runtime_error("Required: <heap_file_name>, csv_file_name, <page_size>.");
    }
    
    
    char* heap_file_path = argv[1];
    char* csv_file_feed = argv[2];
    int page_size = std::atoi(argv[3]);
    if(page_size <= 0) throw std::runtime_error("Invalid page size.");
    
    // LOAD CSV RECORDS.
    
    size_t num_of_attributes;
    char* attr_data = get_attribute_data(csv_file_feed, &num_of_attributes);

    size_t num_records = num_of_attributes / NUMBER_OF_ATTRIBUTES;
    if(num_of_attributes % NUMBER_OF_ATTRIBUTES)
    {
        printf("Error: Inconsistent attribute / record size ratio.\n");
        delete[] attr_data;
        return -1;
    }
    
    int record_size;
    std::vector<Record> records_to_insert =
        extract_record_vector(attr_data, num_records, &record_size);
    
    printf("Inserting %lu records into heap file!\n", num_records);
    
    
    // LOAD HEAP FILE.
    
    Heapfile heapfile;
    heapfile.page_size = page_size;
    heapfile.file_ptr = fopen(heap_file_path, "r+");
    if(!heapfile.file_ptr) throw std::runtime_error("Could not open heapfile!");

    DirectoryEntry* directory = get_directory(&heapfile);
    size_t free_slots = 0;
    for(PageID t = 0; t < NUM_DIRECTORY_ENTRIES; t++)
    {
        free_slots += directory[t].free_slots;
    }
    
    printf("Found %lu free slots.\n", free_slots);
    if(free_slots < num_records)
    {
        clear_records(records_to_insert);
        fclose(heapfile.file_ptr);
        throw std::runtime_error("Not enough space available!");
    }
    
    
    // WRITE!
    int curr_record = 0;
    bool complete = false;
    for(PageID pid = 0; pid < NUM_DIRECTORY_ENTRIES; pid++)
    {
        int free_slots = directory[pid].free_slots;
        if(!free_slots) continue;
        //printf("Found %d free slots on PID %d.\n", free_slots, pid);
        
        Page p;
        init_fixed_len_page(&p, page_size, record_size);
        
        read_page(&heapfile, pid, &p);
        
        int page_change = 0;
        for(int s = 0; s < free_slots; s++)
        {
            int res = add_fixed_len_page(&p, &records_to_insert[curr_record]);
            printf("Record %d added. New RID (%d.%d).\n", curr_record + 1, pid, res);
            curr_record += 1;
            page_change += 1;
            if (curr_record == num_records)
            {
                complete = true;
                break;
            }
        }
        
        //printf("%d slots added to this page.\n", page_change);
        write_page(&p, &heapfile, pid);
        free(p.data);
        if (complete) break;
    }
    
    clear_records(records_to_insert);
    fclose(heapfile.file_ptr);
    return 0;
}
