/*
* Create pm_vheap / Assignment Name
*
* Zengping Xu, Mingxin Zhang / CS5600 / Northeastern University
* Spring 2023 / Mar 19, 2023
*
* In this implementation, we'll build a program-managed virtual heap,
* that allows allocate more memory than the physical memory available  
* we have. We'll use page replacement algorithm FIFO to replacing some
* other page in physical memory when need load page from disk.
*
* Asssume each page size is 1KB, our virtual management system has
* 4KB physical memory, and there are 16KB space in our backing store
* on the disk. To simplifying this practicum, we're allowe to assume
* the maximum amount of memory that can be allocated at a time is
* equal to a single page.
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include "pm_vheap.h"

// Mutex for thread-safe operations
pthread_mutex_t vm_mutex = PTHREAD_MUTEX_INITIALIZER;

// Page table for virtual memory
pte_t page_table[NUM_PAGES];

// Physical memory array
frame_t physical_memory[NUM_FRAMES];

// List of free frames in physical memory
int free_frame_list[NUM_FRAMES];

// Initialize a FIFO Queue
int fifo_queue[NUM_FRAMES];

// Head and tail of FIFO queue (used for page replacement algorithm)
int fifo_queue_head = 0;
int fifo_queue_tail = 0;

// Initialize virtual memory page table by setting all entries to invalid.
void init_page_table() {
    for (int i = 0; i < NUM_PAGES; i++) {
        page_table[i].valid = false;
        page_table[i].frame_num = -1;
        page_table[i].dirty = false;
        page_table[i].referenced = false;
    }
}

// Initialize physical memory and free frame list
void init_physical_memory() {
    for (int i = 0; i < NUM_FRAMES; i++) {
        physical_memory[i].page_num = -1;
        free_frame_list[i] = i;
    }
}

// Initializes the disk by creating a swap file with empty pages.
void init_disk() {
    FILE *swap_file = fopen(SWAP_FILE_NAME, "wb");
    if (swap_file == NULL) {
        perror("Error creating swap file");
        exit(EXIT_FAILURE);
    }
    char empty_page[PAGE_SIZE] = {0};
    for (int i = 0; i < NUM_DISK_PAGES; i++) {
        fwrite(empty_page, sizeof(char), PAGE_SIZE, swap_file);
    }
    fclose(swap_file);
}

// Deallocates a page in the virtual memory page table by setting the entry to invalid.
// Parameters:
//   page_num - the page number to be deallocated
void deallocate_page(int page_num) {
    page_table[page_num].valid = false;
    page_table[page_num].frame_num = -1;
    page_table[page_num].dirty = false;
    page_table[page_num].referenced = false;
}

// Loads a page from disk into physical memory.
// Parameters:
//   page_num - the page number to be loaded from disk
//   frame_num - the frame number in physical memory to load the page into
void load_page(int page_num, int frame_num) {
    FILE *swap_file = fopen(SWAP_FILE_NAME, "rb");
    if (swap_file == NULL) {
        perror("Error opening swap file");
        exit(EXIT_FAILURE);
    }
    // Find the offset in the disk array corresponding to the requested page
    int disk_offset = page_num * PAGE_SIZE;
    fseek(swap_file, disk_offset, SEEK_SET);
    // Read the data from the disk into the physical memory frame
    fread(physical_memory[frame_num].data, sizeof(char), PAGE_SIZE, swap_file);
    fclose(swap_file);

    // Update the page table entry for the loaded page
    page_table[page_num].valid = true;
    page_table[page_num].frame_num = frame_num;
    page_table[page_num].dirty = false;
    page_table[page_num].referenced = true;

    // Update the frame information for the loaded page
    physical_memory[frame_num].page_num = page_num;
}

// Saves a page from physical memory to disk.
// Parameters:
//   page_num - the page number to be saved to disk
//   frame_num - the frame number in physical memory containing the page to save
void save_page(int page_num, int frame_num) {
    FILE *swap_file = fopen(SWAP_FILE_NAME, "rb+");
    if (swap_file == NULL) {
        perror("Error opening swap file");
        exit(EXIT_FAILURE);
    }
    // Find the offset in the disk array corresponding to the requested page
    int disk_offset = page_num * PAGE_SIZE;

    fseek(swap_file, disk_offset, SEEK_SET);
    // Write the data from the physical memory frame to the disk
    fwrite(physical_memory[frame_num].data, sizeof(char), PAGE_SIZE, swap_file);
    fclose(swap_file);

    // Update the page table entry for the saved page
    deallocate_page(page_num);
    // Update the frame information for the saved page
    physical_memory[frame_num].page_num = -1;
}

// Finds the first available free frame in the physical memory.
// Returns the frame number if a free frame is found, or -1 if no free frames are available.
int find_free_frame() {
    if (free_frame_list[0] != -1) {
        int free_frame_num = free_frame_list[0];
        for (int i = 0; i < NUM_FRAMES - 1; i++) {
            free_frame_list[i] = free_frame_list[i + 1];
        }
        free_frame_list[NUM_FRAMES - 1] = -1;
        return free_frame_num;
    }
    return -1; // no free frames available
}

// Adds a page to the FIFO queue for the page replacement algorithm.
// Parameters:
//   page_num - the page number to be added to the FIFO queue
void add_to_fifo_queue(int page_num) {
    page_table[page_num].referenced = true;
    fifo_queue[fifo_queue_tail] = page_num;
    fifo_queue_tail = (fifo_queue_tail + 1) % NUM_FRAMES;
}

// Removes the oldest page from the FIFO queue for the page replacement algorithm.
// Returns the removed page number.
int remove_from_fifo_queue() {
    int page_num = fifo_queue[fifo_queue_head];
    fifo_queue[fifo_queue_head] = -1;
    fifo_queue_head = (fifo_queue_head + 1) % NUM_FRAMES;
    return page_num;
}

// Implements the page replacement algorithm using the First-In, First-Out (FIFO) method.
// Evicts the oldest page from the FIFO queue and returns its frame number.
int page_replacement_fifo() {
    int page_num = remove_from_fifo_queue();
    int frame_num = page_table[page_num].frame_num;
    printf("remove page %d from FIFO queue, it's frame num is %d\n", page_num, frame_num);
    if (page_table[page_num].dirty) {
        save_page(page_num, frame_num);
    }

    // Invalidate the evicted page in the page table
    deallocate_page(page_num);
    return frame_num;
}

// Allocates a frame for a page in the physical memory. If no free frames are available,
// it calls the page_replacement_fifo() function to evict a page and use its frame.
// Parameters:
//   page_num - the page number for which a frame needs to be allocated
// Returns the frame number allocated to the page.
int allocate_frame(int page_num) {
    int frame_num = find_free_frame();
    if (frame_num == -1) {
        printf("no free frame in physical memory for page %d, need page replacemnt\n", page_num);
        frame_num = page_replacement_fifo();
    }
    load_page(page_num, frame_num);
    add_to_fifo_queue(page_num);
    return frame_num;
}

// Reads data from a virtual memory address into a buffer.
// Parameters:
//   address - the virtual memory address to read data from
//   buffer - the buffer to store the read data
//   s - the size of the data to read (in bytes)
void read_data(int address, char *buffer, int s) {
    int size = PAGE_SIZE;
    int page_num = address / PAGE_SIZE;
    int page_offset = address % PAGE_SIZE;
    int frame_num = page_table[page_num].frame_num;
    // Check if the requested page is in physical memory
    if (page_table[page_num].valid) {
        // Page hit
        printf("Page success: find page %d in physical memory\n", page_num);
        memcpy(buffer, &physical_memory[frame_num].data, size);
        page_table[page_num].referenced = true;
    } else {
        // Page fault
        printf("Page Fault: page %d not in physical memory\n", page_num);
        frame_num = allocate_frame(page_num);
        memcpy(buffer, &physical_memory[frame_num].data, size);
    }
}

// Writes data to a virtual memory address from a buffer.
// Parameters:
//   address - the virtual memory address to write data to
//   buffer - the buffer containing the data to be written
//   s - the size of the data to write (in bytes)
void write_data(int address, char *buffer, int s) {
    int size = PAGE_SIZE;
    int page_num = address / PAGE_SIZE;
    int page_offset = address % PAGE_SIZE;
    int frame_num = page_table[page_num].frame_num;

    // Check if the requested page is in physical memory
    if (page_table[page_num].valid) {
        // Page hit
        printf("Page success: find page %d in physical memory\n", page_num);
        memcpy(&physical_memory[frame_num].data, buffer, size);
        page_table[page_num].dirty = true;
        page_table[page_num].referenced = true;
    } else {
        // Page fault
        printf("Page Fault: page %d not in physical memory\n", page_num);
        frame_num = allocate_frame(page_num);
        memcpy(&physical_memory[frame_num].data, buffer, size);
        page_table[page_num].dirty = true;
    }
}

// Allocates a page in virtual memory by finding an invalid entry in the page table
// and loading the corresponding page into a frame in the physical memory. In this
// implementation, we'll allocate one page for each single call. 
// Returns the allocated page number, or -1 if no free pages are available.
int allocate_page() {
    for (int i = 0; i < NUM_PAGES; i++) {
        if (!page_table[i].valid) {
            int frame_num = allocate_frame(i);
            if (frame_num != -1) {
                page_table[i].valid = true;
                page_table[i].frame_num = frame_num;
                page_table[i].dirty = false;
                page_table[i].referenced = false;
                return i;
            }
        }
    }
    return -1; // no free pages available
}

int main() {
    // Initialize the virtual memory system
    init_page_table();
    init_physical_memory();
    init_disk();

    // Allocate some pages in virtual memory
    int page0 = allocate_page();
    printf("success allocate page %d\n", page0);

    int page1 = allocate_page();
    printf("success allocate page %d\n", page1);

    int page2 = allocate_page();
    printf("success allocate page %d\n", page2);

    int page3 = allocate_page();
    printf("success allocate page %d\n", page3);

    // Write some data to the allocated pages
    write_data(page0 * PAGE_SIZE, "Hello, World!", 14);
    printf("success write data to page %d\n", page0);
    write_data(page1 * PAGE_SIZE, "This is page 1.", 16);
    printf("success write data to page %d\n", page1);
    write_data(page2 * PAGE_SIZE, "Another page, page 2.", 21);
    printf("success write data to page %d\n", page2);
    write_data(page3 * PAGE_SIZE, "Page 3 is here!", 15);
    printf("success write data to page %d\n\n", page3);

    printf("Now The Physical Memory is FULL, Let's Allocate a New Page!!\n\n");
    int page4 = allocate_page();
    printf("success allocate page %d\n", page4);
    write_data(page4 * PAGE_SIZE, "The last page, page 4.", 22);
    printf("success write data to page %d\n\n", page4);

    // Read and print the data from the allocated pages
    char buffer[PAGE_SIZE];

    printf("Let's Read data from page0, some replacement will occur!!\n");
    read_data(page0 * PAGE_SIZE, buffer, 14);
    printf("Data in page0: %s\n\n", buffer);

    printf("Let's Read data from page1, some replacement will occur!!\n");
    read_data(page1 * PAGE_SIZE, buffer, 16);
    printf("Data in page1: %s\n\n", buffer);

    printf("Let's Read data from page2, some replacement will occur!!\n");
    read_data(page2 * PAGE_SIZE, buffer, 21);
    printf("Data in page2: %s\n\n", buffer);

    printf("Let's Read data from page3, some replacement will occur!!\n");
    read_data(page3 * PAGE_SIZE, buffer, 15);
    printf("Data in page3: %s\n\n", buffer);

    printf("Let's Read data from page4, some replacement will occur!!\n");
    read_data(page4 * PAGE_SIZE, buffer, 22);
    printf("Data in page4: %s\n\n", buffer);

    return 0;
}

