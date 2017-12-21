#include <vector>
#include <stdio.h>
#include <string>

// RECORD MANAGEMENT.

typedef const char* V;
typedef std::vector<V> Record;

static const int ATTRIBUTE_SIZE = 10;
static const int NUMBER_OF_ATTRIBUTES = 100;

/**
 * Compute the number of bytes required to serialize 'record'.
 */
int fixed_len_sizeof(Record *record);

/**
 * Serialize 'record' to a byte array to be stored in 'buf'.
 */
void fixed_len_write(Record *record, void *buf);

/**
 * Deserializes 'size' bytes from the buffer 'buf', and
 * stores the record in 'record'.
 */
void fixed_len_read(void *buf, int size, Record *record);



// PAGE MANAGEMENT.

typedef struct {
    void *data;
    int page_size;
    int slot_size;
} Page;


/**
 * Initializes a page using the given slot size
 */
void init_fixed_len_page(Page *page, int page_size, int slot_size);

/**
 * Calculates the maximal number of records that fit in a page
 */
int fixed_len_page_capacity(Page *page);

/**
 * Calculate the free space (number of free slots) in the page
 */
int fixed_len_page_freeslots(Page *page);

/**
 * Add a record to the page
 * Returns:
 *   record slot offset if successful,
 *   -1 if unsuccessful (page full)
 */
int add_fixed_len_page(Page *page, Record *r);

/**
 * Write a record into a given slot.
 */
void write_fixed_len_page(Page *page, int slot, Record *r);

/**
 * Read a record from the page from a given slot into 'r'.
 */
void read_fixed_len_page(Page *page, int slot, Record *r);

/**
 * Get the byte-level sum of the contents of a slot.
 */
int get_slot_data_sum(char* slot_bytes, int slot_size);



// HEAP FILE MANAGEMENT.

typedef int PageID;

typedef struct {
    FILE *file_ptr;
    int page_size;
} Heapfile;

typedef struct {
    int offset;
    int free_slots;
} DirectoryEntry;

typedef struct {
    int page_id;
    int slot;
} RecordID;

/**
 * Max pages supported by heap file.
 */
static const int NUM_DIRECTORY_ENTRIES = 2000;
/**
 * A single directory entry needs to store a page offset and a number of freeslots.
 */
static const int DIRECTORY_ENTRY_SIZE = sizeof(DirectoryEntry);
static const int DIRECTORY_PAGE_SIZE = NUM_DIRECTORY_ENTRIES*DIRECTORY_ENTRY_SIZE;

/**
 * Initalize a heapfile to use the file and page size given.
 */
void init_heapfile(Heapfile *heapfile, int page_size, FILE *file);

/**
 * Allocate another page in the heapfile.  This grows the file by a page.
 */
PageID alloc_page(Heapfile *heapfile);

/**
 * Read a page into memory
 */
void read_page(Heapfile *heapfile, PageID pid, Page *page);

/**
 * Write a page from memory to disk
 */
void write_page(Page *page, Heapfile *heapfile, PageID pid);

/**
 * Get a byte array of the page containing the directory.
 * Move the file pointer towards the first page offset!
 */
DirectoryEntry* get_directory(Heapfile *heapfile);

// HELPER FUNCTIONS.

/**
 * Return a byte array with all the attributes in the CSV file.
 * Get rid of the attribute separators.
 */
char* get_attribute_data(std::string csv_file_path, size_t* num_attributes);

/**
 * Create a vector of records using the attributes byte data.
 * Store the size of a record on the variable pointed by 'single_record_size'.
 */
std::vector<Record> extract_record_vector(char* attr_data, size_t num_records, int* single_record_size);

/**
 * Return a vector of pages created from a vector of records.
 */
std::vector<Page> paginate_records(std::vector<Record> records, int page_size, int record_size);

/**
 * Clear all the memory occupied by the records and their attributes.
 */
void clear_records(std::vector<Record> records);

/**
 * Print a comma separated string of the attributes in record to stdout.
 */
void print_record(Record r);


// INTERNAL CLASSES.

class RecordIterator {
private:
    //In current directory.
    DirectoryEntry* directory;
    Heapfile *internal_heapfile;
    Page current_page;
    Page next_page_data;
    int page_ind;
    int next_valid_page;
    int record_ind;
    int next_valid_record;
    int record_capacity;
    bool pages_init;
    bool is_last_slot_empty;
    bool allocation_enabled;
    static const int record_size = NUMBER_OF_ATTRIBUTES*ATTRIBUTE_SIZE;
    void pointToNextPage();
    
public:
    RecordIterator(Heapfile *heapfile);
    ~RecordIterator();
    PageID getLastPageIDRead();
    PageID getLastSlotRead();
    Page getLastPageRead();
    bool isLastSlotEmpty();
    void setAllocationEnabled(bool enabled);
    Record next();
    bool hasNext();
};


// COLUMN STORE FUNCTIONS.

/**
 * Extract an attribute from all records and add them to their own records..
 */
Record build_record_for_column(std::vector<Record> all_records, int attr_id);

/**
 * Slice vector.
 */
Record slice(const Record& v, size_t start=0, size_t end=-1);

/**
 * Print attributes in page, return number of non-empty ones.
 */
size_t print_attributes_in_page(Page* read, Page* project, int page_num, int project_id,
                                int from, int to, std::string start, std::string end);

// OTHER FUNCTIONS.

/**
 * Throw an invalid argument exception.
 */
void throw_invalid_arg();
