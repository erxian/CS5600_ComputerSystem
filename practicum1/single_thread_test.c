#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include "pm_vheap.h"

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

    printf("free page0, since page0 is not in physical memory, warrning occurs\n");
    free_page(page0);
    free_page(page1);
    printf("success free page1\n");
    free_page(page2);
    printf("success free page2\n");
    free_page(page3);
    printf("success free page3\n");
    free_page(page4);
    printf("success free page4\n");

    return 0;
}
