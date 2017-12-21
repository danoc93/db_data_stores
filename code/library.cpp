//
//  library.cpp
//  CSC443-A2
//
//  Created by Daniel Ortiz Costa on 2017-11-14.
//  Copyright © 2017 Daniel Ortiz Costa. All rights reserved.
//

#include "library.hpp"
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <cstring>
#include <stdlib.h>

/***
 RECORD MANAGEMENT. 
 ***/


int fixed_len_sizeof(Record *record)
{
    if (!record) return 0;
    return static_cast<int>(record->size()) * ATTRIBUTE_SIZE;
}


void fixed_len_write(Record *record, void *buf)
{
    if(!record || !buf) throw_invalid_arg();
    
    char* buffer = static_cast<char*>(buf);
    
    for (int i = 0; i < record->size(); i++)
    {
        std::memcpy(buffer + i*ATTRIBUTE_SIZE, (*record)[i], ATTRIBUTE_SIZE);
    }
}


void fixed_len_read(void *buf, int size, Record *record)
{
    int attr_to_write = size / ATTRIBUTE_SIZE;
    if (!record || !buf || size <= 0 || attr_to_write != record->size()) throw_invalid_arg();
    
    char* buffer = static_cast<char*>(buf);

    for (int i = 0; i < attr_to_write; i++)
    {
        char* attribute = new char[ATTRIBUTE_SIZE];
        std::memcpy(attribute, buffer + i*ATTRIBUTE_SIZE, ATTRIBUTE_SIZE);
        (*record)[i] = attribute;
    }
}



/***
 PAGE MANAGEMENT. 
 ***/


void init_fixed_len_page(Page *page, int page_size, int slot_size)
{
    if(!page || page_size <= 0 || slot_size <= 0) throw_invalid_arg();
    
    page->page_size = page_size;
    page->slot_size = slot_size;
    page->data = calloc(1, page_size);
}

           
int fixed_len_page_capacity(Page *page)
{
    if(!page) throw_invalid_arg();
    return page->page_size / page->slot_size;
}


int fixed_len_page_freeslots(Page *page)
{
    if(!page) throw_invalid_arg();
    
    int slot_size = (*page).slot_size;
    char* tmp_record = new char[slot_size];
    char* page_data = static_cast<char*>(page->data);
    int capacity = fixed_len_page_capacity(page);
    
    int slots_utilized = 0;
    for (int i = 0; i < capacity; i++)
    {
        std::memcpy(tmp_record, page_data + i*slot_size, slot_size);
        
        // Validate the record. Skip if empty.
        if(!get_slot_data_sum(tmp_record, slot_size)) continue;
        slots_utilized += 1;
    }
    
    delete[] tmp_record;
    return capacity - slots_utilized;
}


int add_fixed_len_page(Page *page, Record *r)
{
    // Sanity check.
    int slot_size = page->slot_size;
    if(!page || !r || slot_size != fixed_len_sizeof(r)) throw_invalid_arg();
    
    if(!fixed_len_page_freeslots(page)) return -1;
    
    char* tmp_slot = new char[slot_size];
    char* page_data = static_cast<char*>(page->data);
    int capacity = fixed_len_page_capacity(page);
    
    int slot_id;
    for (slot_id = 0; slot_id < capacity; slot_id++)
    {
        std::memcpy(tmp_slot, page_data + slot_id*slot_size, slot_size);
        
        // Occupied.
        if(get_slot_data_sum(tmp_slot, slot_size)) continue;
        
        page_data += slot_id*slot_size;
        for(int i = 0; i < NUMBER_OF_ATTRIBUTES; i++)
        {
           std::memcpy(page_data + i*ATTRIBUTE_SIZE, (*r)[i] , ATTRIBUTE_SIZE);
        }
        break;
    }
    
    delete[] tmp_slot;
    return slot_id;
}


void write_fixed_len_page(Page *page, int slot, Record *r)
{
    int slot_size = page->slot_size;
    if(!page || !r || slot < 0 ||
       slot > fixed_len_page_capacity(page) || slot_size != fixed_len_sizeof(r))
    {
       throw_invalid_arg();
    }
    
    char* page_offset = static_cast<char*>(page->data) + slot*slot_size;
    fixed_len_write(r, page_offset);
}


void read_fixed_len_page(Page *page, int slot, Record *r)
{
    int slot_size = page->slot_size;
    if(!page || !r || slot < 0 || slot > fixed_len_page_capacity(page))
    {
        throw_invalid_arg();
    }
    
    char* page_offset = static_cast<char*>(page->data) + slot*slot_size;
    fixed_len_read(page_offset, slot_size, r);
}


int get_slot_data_sum(char* slot_bytes, int slot_size)
{
    int slot_data_sum = 0;
    for (int j = 0; j < slot_size; j++)
    {
        slot_data_sum |= slot_bytes[j];
    }
    return slot_data_sum;
}



/***
 HEAP FILE FUNCTIONS.
 ***/

void init_heapfile(Heapfile *heapfile, int page_size, FILE *file)
{
    if (!heapfile || page_size <= 0 || !file) throw_invalid_arg();
    
    heapfile->file_ptr = file;
    heapfile->page_size = page_size;
    
    // Add the empty directory page.
    int record_size = ATTRIBUTE_SIZE * NUMBER_OF_ATTRIBUTES;
    DirectoryEntry* directory = new DirectoryEntry[NUM_DIRECTORY_ENTRIES]();
    for(PageID i = 0; i < NUM_DIRECTORY_ENTRIES; i++)
    {
        DirectoryEntry e;
        e.free_slots = heapfile->page_size / record_size;
        e.offset = i*heapfile->page_size;
        directory[i] = e;
    }
    
    fseek(heapfile->file_ptr, 0, std::ios::beg);

    fwrite(directory, DIRECTORY_PAGE_SIZE, 1, heapfile->file_ptr);
    delete[] directory;
    
    fseek(heapfile->file_ptr, 0, std::ios::beg);
}


PageID alloc_page(Heapfile *heapfile)
{
    if (!heapfile || !heapfile->file_ptr) throw_invalid_arg();
    
    DirectoryEntry directory_entry;
    PageID page_id;
    for (page_id = 0; page_id < NUM_DIRECTORY_ENTRIES; page_id++)
    {
        if(!fread(&directory_entry, DIRECTORY_ENTRY_SIZE, 1, heapfile->file_ptr))
        {
            throw std::runtime_error("Heap file is not open!");
        }
        // Find an empty slot in the directory.
        if (directory_entry.free_slots == 0) continue;
        break;
    }
    
    if(page_id == NUM_DIRECTORY_ENTRIES)
    {
        throw std::runtime_error("No free directory entries!");
    }
    
    int record_size = ATTRIBUTE_SIZE * NUMBER_OF_ATTRIBUTES;
    
    // Update the corresponding directory entry.
    DirectoryEntry wrt;
    wrt.free_slots = heapfile->page_size / record_size;
    wrt.offset = page_id*heapfile->page_size;
    
    fseek(heapfile->file_ptr, -DIRECTORY_ENTRY_SIZE, std::ios::cur);
    fwrite(&wrt, DIRECTORY_ENTRY_SIZE, 1, heapfile->file_ptr);
    
    // Go to the EOF.
    fseek(heapfile->file_ptr, 0, std::ios::end);
    char* new_page = new char[heapfile->page_size]();
    fwrite(new_page, 1, heapfile->page_size, heapfile->file_ptr);
    delete[] new_page;
    
    fseek(heapfile->file_ptr, 0, std::ios::beg);
    
    return page_id;
}


void read_page(Heapfile *heapfile, PageID pid, Page *page)
{
    if (!heapfile || !page ||
        pid < 0 || pid >= NUM_DIRECTORY_ENTRIES) throw_invalid_arg();
    
    DirectoryEntry entry;
    fseek(heapfile->file_ptr, DIRECTORY_ENTRY_SIZE*pid, std::ios::beg);
    size_t r = fread(&entry, DIRECTORY_ENTRY_SIZE, 1, heapfile->file_ptr);
    if(!r) throw std::runtime_error("Could not read directory entry!");
    
    fseek(heapfile->file_ptr, 0, std::ios::beg);
    fseek(heapfile->file_ptr, DIRECTORY_PAGE_SIZE + entry.offset, std::ios::cur);
    r = fread(page->data, heapfile->page_size, 1, heapfile->file_ptr);
    if(!r) throw std::runtime_error("Could not read page!");
    
    fseek(heapfile->file_ptr, 0, std::ios::beg);
}


void write_page(Page *page, Heapfile *heapfile, PageID pid)
{
    if (!heapfile || !page ||
        pid < 0 || pid >= NUM_DIRECTORY_ENTRIES) throw_invalid_arg();
    
    // Load -> Update count -> Re-write.
    DirectoryEntry entry;
    fseek(heapfile->file_ptr, DIRECTORY_ENTRY_SIZE*pid, std::ios::beg);
    size_t r = fread(&entry, DIRECTORY_ENTRY_SIZE, 1, heapfile->file_ptr);
    if(!r) throw std::runtime_error("Could not read directory entry!");
    
    entry.free_slots = fixed_len_page_freeslots(page);
    fseek(heapfile->file_ptr, -DIRECTORY_ENTRY_SIZE, std::ios::cur);
    r = fwrite(&entry, DIRECTORY_ENTRY_SIZE, 1, heapfile->file_ptr);
    if(!r) throw std::runtime_error("Could not update directory entry!");
    
    // Load -> Re-write.
    fseek(heapfile->file_ptr, 0, std::ios::beg);
    fseek(heapfile->file_ptr, DIRECTORY_PAGE_SIZE + entry.offset, std::ios::cur);
    r = fwrite(page->data, heapfile->page_size, 1, heapfile->file_ptr);
    if(!r) throw std::runtime_error("Could not update page!");
    
    fseek(heapfile->file_ptr, 0, std::ios::beg);
}


DirectoryEntry* get_directory(Heapfile *heapfile)
{
    if (!heapfile || !heapfile->file_ptr) throw_invalid_arg();
    DirectoryEntry* directory = new DirectoryEntry[NUM_DIRECTORY_ENTRIES]();
    fseek(heapfile->file_ptr, 0, std::ios::beg);
    fread(directory, DIRECTORY_PAGE_SIZE, 1, heapfile->file_ptr);
    return directory;
}


/***
 HELPER FUNCTIONS.
 ***/


char* get_attribute_data(std::string csv_file_path, size_t* num_attributes)
{
    std::ifstream in_file;
    size_t size;
    
    in_file.open(csv_file_path.c_str(), std::ios::in|std::ios::binary|std::ios::ate);
    if(!in_file.good()) throw std::runtime_error("Could not open source file!");
    
    in_file.seekg(0, std::ios::end);
    size = in_file.tellg();
    in_file.seekg(0, std::ios::beg);
    
    char* csv_data = new char[size];
    in_file.read(csv_data, size);
    in_file.close();
    
    int separator_cnt = 0;
    for ( size_t i = 0; i < size; i++ ) separator_cnt += csv_data[i] == ',';
    
    *num_attributes = (size - separator_cnt)/ATTRIBUTE_SIZE;
    char* attribute_bytes = new char[*num_attributes * ATTRIBUTE_SIZE];
    
    size_t curr_attr = 0;
    size_t curr_point = 0;
    while ( curr_point < size )
    {
        std::memcpy(attribute_bytes + curr_attr*ATTRIBUTE_SIZE, csv_data + curr_point, ATTRIBUTE_SIZE);
        curr_point += ATTRIBUTE_SIZE + 1;
        curr_attr += 1;
    }
    
    printf("File Size: %lu, Attr Size: %d, Attributes Read: %lu\n",
           size, ATTRIBUTE_SIZE, *num_attributes);
    
    delete[] csv_data;
    return attribute_bytes;
}


std::vector<Record> extract_record_vector(char* attr_data, size_t num_records, int* single_record_size)
{
    std::vector<Record> records(num_records);
    for(size_t i = 0; i < num_records; i++)
    {
        Record r(NUMBER_OF_ATTRIBUTES);
        *single_record_size = fixed_len_sizeof(&r);
        fixed_len_read(attr_data + i*(*single_record_size), *single_record_size, &r);
        records[i] = r;
    }
    return records;
}


std::vector<Page> paginate_records(std::vector<Record> records, int page_size, int record_size)
{
    std::vector<Page> pages;
    int ind_r = 0;
    size_t num_records = records.size();
    while(ind_r < num_records)
    {
        Page p;
        init_fixed_len_page(&p, page_size, record_size);
        
        size_t record_capacity =
        std::min((size_t)fixed_len_page_capacity(&p), (num_records - ind_r));
        for(int i = 0; i < record_capacity; i++)
        {
            write_fixed_len_page(&p, i, &(records[ind_r]));
            ind_r += 1;
        }
        pages.push_back(p);
    }
    return pages;
}


void clear_records(std::vector<Record> records)
{
    for(size_t i = 0; i < records.size(); i++)
    {
        Record r = records[i];
        for(int j = 0; j < NUMBER_OF_ATTRIBUTES; j++)
        {
            delete[] r[j];
        }
    }
    records.clear();
}


void print_record(Record r)
{
    if(!r.size())
    {
        std::cout<< "Error: Invalid record!" << std::endl;
        return;
    }
    
    char* buff = new char[ATTRIBUTE_SIZE + 1]();
    for (size_t attr_id = 0; attr_id < r.size(); attr_id++)
    {
        memcpy(buff, r[attr_id], ATTRIBUTE_SIZE);
        printf("%s", buff);
        if (attr_id == r.size() - 1) break;
        printf(",");
    }
    delete[] buff;
    std::cout << std::endl;
}



/***
 RECORD ITERATOR.
 ***/


RecordIterator::RecordIterator(Heapfile *heapfile)
{
    //Load the directory!
    directory = get_directory(heapfile);
    internal_heapfile = heapfile;
    
    init_fixed_len_page(&current_page, heapfile->page_size, record_size);
    init_fixed_len_page(&next_page_data, heapfile->page_size, record_size);
    
    allocation_enabled = false;
    is_last_slot_empty = false;
    pages_init = false;
    page_ind = 0;
    record_ind = 0;
    next_valid_record = -1;
    next_valid_page = -1;
    record_capacity = internal_heapfile->page_size / record_size;
}

RecordIterator::~RecordIterator()
{
    if(!internal_heapfile || !current_page.data) return;
    free(current_page.data);
}

void RecordIterator::setAllocationEnabled(bool enabled)
{
    allocation_enabled = enabled;
}

Page RecordIterator::getLastPageRead()
{
    return next_page_data;
}

void RecordIterator::pointToNextPage()
{
    // If this page is complete, attempt load the next page.
    int p;
    for(p = page_ind; p < NUM_DIRECTORY_ENTRIES; p++)
    {
        DirectoryEntry e = directory[p];
        
        //Unallocated page. May want to do something about this?
        if(e.free_slots == record_capacity)
        {
            if (!allocation_enabled) continue;
        }
        read_page(internal_heapfile, page_ind, &current_page);
        record_ind = 0;
        pages_init = true;
        break;
    }
    page_ind = p;
}


PageID RecordIterator::getLastPageIDRead()
{
    return next_valid_page;
}


PageID RecordIterator::getLastSlotRead()
{
    return next_valid_record;
}


bool RecordIterator::isLastSlotEmpty()
{
    return is_last_slot_empty;
}


bool RecordIterator::hasNext()
{
    // No more pages.
    if(next_valid_record < 0 && page_ind == NUM_DIRECTORY_ENTRIES) return false;
    
    // We have not yet consumed it.
    if(next_valid_record >= 0) return true;
    
    if(!pages_init) pointToNextPage();
    
    while(record_ind < record_capacity)
    {
        char* slot_addr = static_cast<char*>(current_page.data) + record_ind*record_size;
        is_last_slot_empty = !get_slot_data_sum(slot_addr, record_size);
        
        next_valid_record = record_ind;
        next_valid_page = page_ind;
        read_page(internal_heapfile, page_ind, &next_page_data);
        
        if(record_ind == record_capacity - 1)
        {
            page_ind += 1;
            pointToNextPage();
        }
        else
        {
            record_ind += 1;
        }
        
        return true;
    }
    
    // This should not happen unless we got to the point where we found no more pages!
    return !(record_ind == record_capacity);
    
}


Record RecordIterator::next()
{
    if(!hasNext()) throw std::runtime_error("No more records!");
    Record r(NUMBER_OF_ATTRIBUTES);
    read_fixed_len_page(&next_page_data, next_valid_record, &r);
    next_valid_record = -1;
    return r;
}



/***
 COLUMN STORE FUNCTIONS.
 ***/

Record build_record_for_column(std::vector<Record> all_records, int attr_id)
{
    if(attr_id < 0 || attr_id > NUMBER_OF_ATTRIBUTES)
    {
        throw std::runtime_error("Invalid attribute ID");
    }
    
    Record r;
    
    for(size_t e = 0; e < all_records.size(); e++)
    {
        r.push_back(all_records[e][attr_id]);
    }
    return r;
}

size_t print_attributes_in_page(Page* read, Page* project, int page_num, int project_id,
                                int from, int to, std::string start, std::string end)
{
    if(!read || !read->data || !project || !project->data) throw_invalid_arg();
    
    size_t num = 0;
    
    char* val = new char[ATTRIBUTE_SIZE + 1]();
    
    size_t capacity = read->page_size / read->slot_size;
    
    for (size_t i = 0; i < capacity; i++)
    {
        memcpy(val, (char*)(read->data) + i*read->slot_size, read->slot_size);
        if(!get_slot_data_sum(val, read->slot_size)) continue;
        if(start.compare(val) > 0 || end.compare(val) < 0) continue;
        num++;
        
        if(project != read)
        {
            memcpy(val, (char*)(project->data) + i*project->slot_size, project->slot_size);
        }
        
        val[to + 1] = '\0';
        printf("@%d.%lu[%d] -> '%s'\n", page_num, i, project_id, val + from);
    }
    delete[] val;
    return num;
}


/***
 OTHER FUNCTIONS.
 ***/


Record slice(Record& v, size_t start, size_t end)
{
    size_t oldlen = v.size();
    size_t newlen;
    
    if (end == -1 or end >= oldlen){
        newlen = oldlen-start;
    } else {
        newlen = end-start;
    }
    
    Record nv(newlen);
    
    for (int i=0; i<newlen; i++) {
        nv[i] = v[start+i];
    }
    return nv;
}


void throw_invalid_arg()
{
    throw std::invalid_argument("Invalid arguments.");
}
