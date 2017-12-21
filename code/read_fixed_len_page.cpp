//
//  read_fixed_len_page.cpp
//  CSC443-A2
//
//  Created by Daniel Ortiz Costa on 2017-11-15.
//  Copyright © 2017 Daniel Ortiz Costa. All rights reserved.
//

#include "read_fixed_len_page.hpp"
#include <fstream>
#include <iostream>
#include <ctime>
#include <cstdlib>
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Error! Required: <src_page_file>, <page_size>.\n");
        return -1;
    }
    
    std::string src_page_file(argv[1]);
    int page_size = std::atoi(argv[2]);
    
    if(page_size <= 0) throw std::runtime_error("Invalid page size.");
        
    size_t bytes_to_read = get_paged_data_size(src_page_file);
    size_t record_size = NUMBER_OF_ATTRIBUTES * ATTRIBUTE_SIZE;
    size_t num_pages_to_read = bytes_to_read / page_size;
    
    if (bytes_to_read % page_size)
    {
        printf("Error! Inconsistent file size for given page size.\n");
        return -1;
    }
    
    clock_t start = clock();
    
    printf("Reading %lu pages from %s...\n", num_pages_to_read, src_page_file.c_str());
    std::vector<Page> all_pages =
        get_pages_vector(src_page_file, num_pages_to_read, page_size, record_size);
    
    float time_to_read = (float( clock () - start ) /  CLOCKS_PER_SEC) * 1000;
    
    std::vector<Record> all_records = get_all_records(all_pages);
    all_pages.clear();
    
    std::string output_csv_name = src_page_file+".csv";
    //Save them into a file and free them as they are written.
    write_records_to_csv_file(output_csv_name, all_records);
    printf("File %s created.\n\n", output_csv_name.c_str());
    all_records.clear();
    
    printf("NUMBER OF PAGES READ: %lu (%d bytes)\n", num_pages_to_read, page_size);
    printf("TIME ELAPSED: %f ms\n\n", time_to_read);

    return 0;
}


size_t get_paged_data_size(std::string page_file_path)
{
    std::ifstream in_file;
    size_t size;
    
    in_file.open(page_file_path.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
    if(!in_file.good()) throw std::runtime_error("Could not open source file!");
    
    in_file.seekg(0, std::ios::end);
    size = in_file.tellg();
    in_file.seekg(0, std::ios::beg);
    in_file.close();
    
    return size;
}


std::vector<Page> get_pages_vector(std::string page_file_path, size_t num_pages,
                                   int page_size, size_t record_size)
{
    std::ifstream in_file;
    in_file.open(page_file_path.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
    if(!in_file.good()) throw std::runtime_error("Could not open source file!");
    in_file.seekg(0, std::ios::beg);
    
    std::vector<Page> all_pages;
    size_t pages_read = 0;
    while(pages_read < num_pages)
    {
        Page p;
        init_fixed_len_page(&p, page_size, (int)record_size);
        in_file.read(static_cast<char*>(p.data), page_size);
        all_pages.push_back(p);
        pages_read++;
    }
    
    in_file.close();
    return all_pages;
}


std::vector<Record> get_all_records(std::vector<Page> pages)
{
    std::vector<Record> all_records;
    for(size_t i = 0; i < pages.size(); i++)
    {
        Page page = pages[i];
        int capacity = fixed_len_page_capacity(&page);
        char* data_bytes = static_cast<char*>(page.data);
        for(int r = 0; r < capacity; r++)
        {
            if(!get_slot_data_sum(data_bytes + r*page.slot_size, page.slot_size)) continue;
            Record record(NUMBER_OF_ATTRIBUTES);
            read_fixed_len_page(&page, r, &record);
            all_records.push_back(record);
        }
        
        free(page.data);
    }
    
    return all_records;
}


void write_records_to_csv_file(std::string records_file_dst, std::vector<Record> records)
{
    std::ofstream result_file;
    result_file.open(records_file_dst.c_str(), std::ios::binary | std::ios::out);
    if(!result_file.good()) throw std::runtime_error("Could not create destination file!");
    
    for(size_t i = 0; i < records.size(); i++)
    {
        Record r = records[i];
        
        for(size_t a = 0; a < NUMBER_OF_ATTRIBUTES; a++)
        {
            result_file.write(r[a], ATTRIBUTE_SIZE);
            delete[] r[a];
            if (a == NUMBER_OF_ATTRIBUTES - 1 && i == records.size() - 1) continue;
            result_file << ",";
        }
    }
    
    result_file.close();
}
