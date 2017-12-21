
//
//  Perform the selection of an attribute for a col-based storage.
//  select3.cpp
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
    
    if (argc != 7)
    {
        printf("Error! Required: <colstore_basepath> <attribute_id> <return_id> <start> <end> <page_size>.\n");
        return -1;
    }
    
    std::string colstore_base(argv[1]);
    
    int page_size = std::atoi(argv[6]);
    if(page_size <= 0) throw std::runtime_error("Invalid page size.");
    
    int attr_id = std::atoi(argv[2]);
    int return_id = std::atoi(argv[3]);
    if(attr_id < 0 || attr_id > NUMBER_OF_ATTRIBUTES ||
       return_id < 0 || return_id > NUMBER_OF_ATTRIBUTES)
    {
        throw std::runtime_error("Invalid attribute ID.");
    }
    
    std::string START(argv[4]);
    std::string END(argv[5]);
    
    printf("SELECT R[%d][%d:%d] WHERE R[%d] >= '%s' AND R[%d] <= '%s'\n",
           return_id, FROM, TO, attr_id, START.c_str(), attr_id, END.c_str());
    
    if(START.compare(END) > 0) throw std::runtime_error("START > END!");
    
    
    // LOAD COLSTORE DATA.
    
    std::string col_file_name = colstore_base + "/cstore_" + std::to_string(attr_id);
    std::string proj_file_name = colstore_base + "/cstore_" + std::to_string(return_id);
    
    FILE* file_r = NULL;
    FILE* file_p = NULL;
    file_r = fopen(col_file_name.c_str(), "r");
    
    if(col_file_name != proj_file_name)
    {
        file_p = fopen(proj_file_name.c_str(), "r");
        if(!file_p) throw std::runtime_error("Could not read colstore for return_id!");
    }
    
    if(!file_r) throw std::runtime_error("Could not read colstore for attr_id!");
    
    int pages_so_far = 0;
    size_t selected = 0;
    
    while (true)
    {
        Page read_page;
        init_fixed_len_page(&read_page, page_size, ATTRIBUTE_SIZE);
        
        Page project_page;
        if(file_p) init_fixed_len_page(&project_page, page_size, ATTRIBUTE_SIZE);
        
        size_t read_num = fread(read_page.data, page_size, 1, file_r);
        if(!read_num) break;
        
        if(file_p)
        {
            fread(project_page.data, page_size, 1, file_p);
        }
        else
        {
            project_page = read_page;
        }
        
        std::string RID = "." + std::to_string(attr_id);
        selected += print_attributes_in_page(&read_page, &project_page,
                                             pages_so_far, return_id, FROM, TO, START, END);
        free(read_page.data);
        if(file_p) free(project_page.data);
        
        pages_so_far ++;
    }
    
    fclose(file_r);
    if(file_p) fclose(file_p);
    
    printf("Total Tuples = %lu\n", selected);
    return 0;
}
