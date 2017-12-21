#include <stdio.h>
#include "library.hpp"

/**
 * Write page_size'd pages to a file located in page_file_path.
 * Return the time elapsed in ms.
 */
float write_pages(std::vector<Page> pages, std::string page_file_path, int page_size);
