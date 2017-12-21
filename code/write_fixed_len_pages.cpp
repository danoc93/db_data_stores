//
//  Create a page file for the records in a csv file.
//  write_fixed_len_pages.cpp
//  CSC443-A2
//
//  Created by Daniel Ortiz Costa on 2017-11-14.
//  Copyright © 2017 Daniel Ortiz Costa. All rights reserved.
//

#include "write_fixed_len_pages.hpp"
#include <fstream>
#include <iostream>
#include <algorithm> 
#include <ctime>

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Error! Required: <src_csv_file>, <page_file_name>, <page_size>.\n");
        return -1;
    }
    
    std::string csv_file_path(argv[1]);
    std::string file_out(argv[2]);
    int page_size = std::atoi(argv[3]);
    
    if(page_size <= 1) throw std::runtime_error("Invalid page size!");
        
    std::cout << "Reading " << csv_file_path << std::endl;
    size_t num_of_attributes;
    char* attr_data = get_attribute_data(csv_file_path, &num_of_attributes);
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
    
    std::vector<Page> pages = paginate_records(records, page_size, record_size);
    clear_records(records);
    
    //Write pages and delete the contents from memory!
    std::cout << "Attempting to create pagefile..." <<std::endl;
    double time_to_write = write_pages(pages, file_out, page_size);
    std::cout << "Finished creating pagefile " << file_out << std::endl << std::endl;
    
    printf("NUMBER OF RECORDS: %lu (%d bytes)\n", num_records, record_size);
    printf("NUMBER OF PAGES: %lu (%d bytes)\n", pages.size(), page_size);
    printf("TIME ELAPSED: %f ms\n\n", time_to_write);
    
    return 0;
}


float write_pages(std::vector<Page> pages, std::string page_file_path, int page_size)
{
    clock_t start = clock();
    
    std::ofstream result_file;
    result_file.open(page_file_path.c_str(), std::ios::binary | std::ios::out);
    if(!result_file.good()) throw std::runtime_error("Could not create destination file!");
    
    for(int i = 0; i < pages.size(); i++)
    {
        result_file.write(static_cast<char*>(pages[i].data), page_size);
        result_file.flush();
        free(pages[i].data);
    }
    result_file.close();
    pages.clear();
    
    return (float( clock () - start ) /  CLOCKS_PER_SEC) * 1000;
}
