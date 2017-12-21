//
//  Perform the selection of an attribute for a col-based storage.
//  select2.cpp
//  CSC443-A2
//
//  Created by Daniel Ortiz Costa on 2017-11-17.
//  Copyright © 2017 Daniel Ortiz Costa. All rights reserved.
//

#include <stdio.h>
#include <string>
#include <iostream>
#include <math.h>
#include "library.hpp"

int main(int argc, char*argv[])
{
    const int FROM = 1;
    const int TO = 5;
    
    if (argc != 6)
    {
        printf("Error! Required: <colstore_basepath> <attribute_id> <start> <end> <page_size>.\n");
        return -1;
    }
    
    std::string colstore_base(argv[1]);
    
    int page_size = std::atoi(argv[5]);
    if(page_size <= 0) throw std::runtime_error("Invalid page size.");
    
    int attr_id = std::atoi(argv[2]);
    if(attr_id < 0 || attr_id > NUMBER_OF_ATTRIBUTES) throw std::runtime_error("Invalid attribute ID.");
    
    std::string START(argv[3]);
    std::string END(argv[4]);
    
    printf("SELECT R[%d][%d:%d] WHERE R[%d] >= '%s' AND R[%d] <= '%s'\n",
           attr_id, FROM, TO, attr_id, START.c_str(), attr_id, END.c_str());
    
    if(START.compare(END) > 0) throw std::runtime_error("START > END!");
    
    
    // LOAD COLSTORE DATA.
        
    std::string col_file_name = colstore_base + "/cstore_" + std::to_string(attr_id);
    
    FILE* file_r;
    file_r = fopen(col_file_name.c_str(), "r");
    if(!file_r) throw std::runtime_error("Could read colstore for attr_id!");
    
    int pages_so_far = 0;
    size_t selected = 0;
    while (true)
    {
        Page p;
        init_fixed_len_page(&p, page_size, ATTRIBUTE_SIZE);
        size_t read_num = fread(p.data, page_size, 1, file_r);
        std::string RID = "." + std::to_string(attr_id);
        selected += print_attributes_in_page(&p, &p, pages_so_far, attr_id, FROM, TO, START, END);
        free(p.data);
        pages_so_far ++;
        if(!read_num) break;
    }
    
    printf("Total Tuples = %lu\n", selected);
    return 0;
}
