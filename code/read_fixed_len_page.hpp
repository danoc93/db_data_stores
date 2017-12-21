//
//  Read a page file and store its contents in a CSV file.
//  read_fixed_len_page.hpp
//  CSC443-A2
//
//  Created by Daniel Ortiz Costa on 2017-11-15.
//  Copyright © 2017 Daniel Ortiz Costa. All rights reserved.
//

#include <stdio.h>
#include "library.hpp"

/**
 * Return a byte array with all the attributes in the page file.
 */
size_t get_paged_data_size(std::string page_file_path);


/**
 * Create a vector of pages from the page file.
 */
std::vector<Page> get_pages_vector(std::string page_file_path, size_t num_pages,
                                   int page_size, size_t record_size);

/**
 * Extract all available records inside the pages and build a vector.
 * Free the pages as you read them.
 */
std::vector<Record> get_all_records(std::vector<Page> pages);


/**
 * Save all the records to a csv file with the same name as the src.
 * Free the records as they are written!
 */
void write_records_to_csv_file(std::string records_file_dst, std::vector<Record> records);
