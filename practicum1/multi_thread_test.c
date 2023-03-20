#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <string.h>
#include "pm_vheap.h"

#define NUM_THREADS 5 

/**
 * The worker function allocates and frees memory using pm_malloc and pm_free.
 *
 * @param arg A pointer to the worker's ID.
 * @return NULL.
 */
void *worker(void *arg) {
    int id = *(int*)arg;
    printf("Worker %d started\n", id);

    // Allocate page 
    int page = allocate_page();
    printf("success allocate page %d for worker %d\n", page, id);

    // Write data to page
    char message[50];
    snprintf(message, sizeof(message), "Hello, World! from worker %d", id);
    printf("Start to write data to page %d\n", page);
    write_data(page * PAGE_SIZE, message, strlen(message) + 1);
    printf("success write data to page %d\n\n", page);

    // Read and print the data from the allocated pages
    char buffer[PAGE_SIZE];
    printf("Start to read data from page %d\n", page);
    read_data(page * PAGE_SIZE, buffer, strlen(message) + 1);
    printf("Read Data in page%d: %s\n\n", page, buffer);

    // Free memory
    free_page(page);
    printf("success free page %d from worker %d\n", page, id);
    return NULL;
}

int main() {
    // Initialize the heap
    init_page_table();
    init_physical_memory();
    init_disk();

    // Create threads
    pthread_t threads[NUM_THREADS];
    int ids[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        ids[i] = i;
        int result = pthread_create(&threads[i], NULL, worker, &ids[i]);
        if (result != 0) {
            printf("Failed to create thread %d\n", i);
            return 1;
        }
    }

    // Wait for threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}

