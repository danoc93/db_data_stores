//
//  Build a series of column-centric heap files for a CSV file.
//  csv2colstore.cpp
//  CSC443-A2
//
//  Created by Daniel Ortiz Costa on 2017-11-17.
//  Copyright © 2017 Daniel Ortiz Costa. All rights reserved.
//

#include "library.hpp"
#include <stdio.h>
#include <string>
#include <iostream>
#include <math.h>
#include <fstream>
#include <cstring>
#include <cstdlib>
#include <sstream>

int main(int argc, char*argv[])
{
    if (argc != 4)
    {
        printf("Error! Required: <src_csv_file>, <colstore_basepath>, <page_size>.\n");
        return -1;
    }
    
    std::string csv_file_name(argv[1]);
    std::string colstore_base(argv[2]);
    int page_size = std::atoi(argv[3]);
    
    if(page_size < ATTRIBUTE_SIZE)
    {
        printf("Error: Cannot write data to pages. Page Size (%d) < Attribute Size (%d).\n",
               page_size, ATTRIBUTE_SIZE);
        return -1;
    }
    
    // PREPARE CSV DATA.
    
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
    
    
    printf("Spreading {%lu} records across {%d} column stores.\n",
           num_records, NUMBER_OF_ATTRIBUTES);
    
    
    //  CONSTRUCT COL STORES.
    
    size_t capacity_required = num_records * ATTRIBUTE_SIZE;
    size_t pages_required = ceil(capacity_required / (float)page_size);
    size_t page_attr_capacity = page_size / ATTRIBUTE_SIZE;
    
    printf("Pages required/colstore -> %lu | attr_capacity/page -> %lu\n",
           pages_required, page_attr_capacity);
    
    for (int i = 0; i < NUMBER_OF_ATTRIBUTES; i++)
    {
        Record r = build_record_for_column(records, i);
        Record::iterator it = r.begin();
        
        std::string col_file_name = colstore_base + "/cstore_" + std::to_string(i);
        std::ofstream file_r;
        file_r.open(col_file_name, std::ios::binary | std::ios::out);
        if(!file_r.good()) throw std::runtime_error("Could not create destination file!");
        
        for(PageID pid = 0; pid < pages_required; pid++)
        {
            Record page_attributes;
            int count = 0;
            while (it != r.end())
            {
                page_attributes.push_back(*it);
                count++;
                it++;
                if(count == page_attr_capacity) break;
            }
            
            Page p;
            int slot_size = (int)page_attributes.size() * ATTRIBUTE_SIZE;
            init_fixed_len_page(&p, page_size, slot_size);
            write_fixed_len_page(&p, 0, &page_attributes);
            
            file_r.write(static_cast<char*>(p.data), page_size);
            file_r.flush();
            
            free(p.data);
        }
        
        file_r.close();
    }
    
    std::cout << "Store added to " << colstore_base << std::endl;
    
    clear_records(records);
    return 0;
}
