#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>

#define PAGE_SIZE 1024
#define PHYSICAL_MEMORY_SIZE 4094 
#define DISK_MEMORY_SIZE 16384
#define SWAP_FILE "swap_file.bin"
#define N (PHYSICAL_MEMORY_SIZE / PAGE_SIZE)

typedef struct Page {
    int id;
    char data[PAGE_SIZE];
    bool available;
} Page;

Page phys_mem[N];
bool available_pages[DISK_MEMORY_SIZE / PAGE_SIZE];

int lru_count[N];

int num_pages = 0;
int disk_page_count = 0;

void init_memory() {
    for (int i = 0; i < N; i++) {
        phys_mem[i].id = -1;
        lru_count[i] = 0;
    }

    for (int i = 0; i < (DISK_MEMORY_SIZE / PAGE_SIZE); i++) {
        available_pages[i] = false;
    }
}

int allocate_page(const char *data) {
    int id = -1;
    for (int i = 0; i < (DISK_MEMORY_SIZE / PAGE_SIZE); i++) {
        if (available_pages[i] == true) {
            id = i;
            available_pages[i] = false;
            break;
        }
    }

    if (id == -1) {
        id = ++num_pages;
    }

    Page new_page;
    new_page.id = id;
    strcpy(new_page.data, data);
    new_page.available = false;

    FILE *swap_file = fopen(SWAP_FILE, "r+b");
    if (swap_file == NULL) {
        perror("Error opening swap file");
        exit(EXIT_FAILURE);
    }

    fseek(swap_file, id * sizeof(Page), SEEK_SET);
    fwrite(&new_page, sizeof(Page), 1, swap_file);
    fclose(swap_file);
    disk_page_count++;

    return id;
}

void free_page(int id) {
    available_pages[id] = true;

    for (int i = 0; i < N; i++) {
        if (phys_mem[i].id == id) {
            phys_mem[i].id = -1;
            lru_count[i] = 0;
            break;
        }
    }

    FILE *swap_file = fopen(SWAP_FILE, "r+b");
    if (swap_file == NULL) {
        perror("Error opening swap file");
        exit(EXIT_FAILURE);
    }

    Page empty_page;
    empty_page.id = id;
    memset(empty_page.data, 0, PAGE_SIZE);
    empty_page.available = true;

    fseek(swap_file, id * sizeof(Page), SEEK_SET);
    fwrite(&empty_page, sizeof(Page), 1, swap_file);
    fclose(swap_file);
}


void load_page(int id) {
    int min_lru = INT_MAX;
    int min_lru_index = -1;

    for (int i = 0; i < N; i++) {
        if (phys_mem[i].id == id) {
            lru_count[i] = 0;
            printf("found page %d in physical memory\n", id);
            return;
        }

        if (lru_count[i] < min_lru) {
            min_lru = lru_count[i];
            min_lru_index = i;
        }
    }

    FILE *swap_file = fopen(SWAP_FILE, "rb");
    if (swap_file == NULL) {
        perror("Error opening swap file");
        exit(EXIT_FAILURE);
    }

    Page temp;
    while (fread(&temp, sizeof(Page), 1, swap_file)) {
        if (temp.id == id) {
            printf("load page %d from disk and replace the Least Recently Used page %d\n", id, phys_mem[min_lru_index].id);
            memcpy(phys_mem[min_lru_index].data, temp.data, PAGE_SIZE);
            phys_mem[min_lru_index].id = id;
            lru_count[min_lru_index] = 0;
            break;
        }
    }
    fclose(swap_file);
}

void access_memory(int id, char *data) {
    load_page(id);

    for (int i = 0; i < N; i++) {
        if (phys_mem[i].id == id) {
            strcpy(data, phys_mem[i].data);
            break;
        }
    }

    for (int i = 0; i < N; i++) {
        if (phys_mem[i].id != -1) {
            lru_count[i]++;
        }
    }
}


int main() {
    init_memory();

    char data[PAGE_SIZE];

    int page1 = allocate_page("Hello, World!");
    printf("allocate page %d\n", page1);
    access_memory(page1, data);
    printf("Data in page1: %s\n", data);

    int page2 = allocate_page("This is page 2.");
    access_memory(page2, data);
    printf("Data in page2: %s\n", data);

    access_memory(page1, data);
    printf("Data in page1: %s\n", data);

    int page3 = allocate_page("Another page, page 3.");
    access_memory(page3, data);
    printf("Data in page3: %s\n", data);

    int page4 = allocate_page("Page 4 is here!");
    access_memory(page4, data);
    printf("Data in page4: %s\n", data);

    int page5 = allocate_page("The last page, page 5.");
    access_memory(page5, data);
    printf("Data in page5: %s\n", data);

    free_page(1);
    free_page(2);
    free_page(3);
    free_page(4);
    free_page(5);

    return 0;
}

