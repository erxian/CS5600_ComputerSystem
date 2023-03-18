#ifndef VIRTUAL_MEMORY_H
#define VIRTUAL_MEMORY_H

#include <stdbool.h>

// Constants
#define NUM_FRAMES 4 // number of physical memory frames
#define PAGE_SIZE 1024 // size of each page
#define NUM_PAGES 16 // total number of pages in virtual memory
#define DISK_SIZE 16384 // size of the backing store disk in bytes
#define NUM_DISK_PAGES (DISK_SIZE / PAGE_SIZE) // number of pages on disk
#define SWAP_FILE_NAME "swap_file.bin" // swap file

// Type definitions
// Physical memory frame
typedef struct frame_t {
    int page_num; // page number of the page currently in the frame (-1 if empty)
    char data[PAGE_SIZE]; // actual data stored in the frame
} frame_t;

// Virtual memory page table entry
typedef struct pte_t {
    bool valid; // true if the page is currently in physical memory
    int frame_num; // index of the physical memory frame containing the page
    bool dirty; // true if the page has been modified since it was loaded into physical memory
    bool referenced; // true if the page has been referenced recently (used for page replacement algorithm)
} pte_t;

// Function prototypes
void init_page_table();
void init_physical_memory();
void init_disk();
void deallocate_page(int page_num);
void load_page(int page_num, int frame_num);
void save_page(int page_num, int frame_num);
int find_free_frame();
void add_to_fifo_queue(int page_num);
int remove_from_fifo_queue();
int page_replacement_fifo();
int allocate_frame(int page_num);
void read_data(int address, char *buffer, int s);
void write_data(int address, char *buffer, int s);
int allocate_page();

#endif // VIRTUAL_MEMORY_H
