#include <stdio.h>
#include <stdlib.h>

#define PAGE_TABLE_SIZE 32
#define FRAME_SIZE 16
#define MEMORY_SIZE (PAGE_TABLE_SIZE * FRAME_SIZE)

typedef struct {
    int valid;
    int frame_number;
    int counter;
} PageTableEntry;

typedef struct {
    PageTableEntry page_table[PAGE_TABLE_SIZE];
    char ram[MEMORY_SIZE];
    int page_faults;
} MemoryManager;

int find_free_frame(PageTableEntry page_table[]) {
    for (int i = 0; i < PAGE_TABLE_SIZE; ++i) {
        if (!page_table[i].valid) {
            return i;
        }
    }
    return -1;
}

int least_recently_used(PageTableEntry page_table[]) {
    int min_counter = page_table[0].counter;
    int index_to_replace = 0;

    for (int i = 1; i < PAGE_TABLE_SIZE; ++i) {
        if (page_table[i].counter < min_counter) {
            min_counter = page_table[i].counter;
            index_to_replace = i;
        }
    }

    return index_to_replace;
}

void handle_page_fault(MemoryManager *manager, char *process_file_name, int logical_address) {
    manager->page_faults++;

    int page_number = logical_address / FRAME_SIZE;
    int frame_number = find_free_frame(manager->page_table);

    if (frame_number == -1) {
        frame_number = least_recently_used(manager->page_table);
    }

    FILE *process_file = fopen(process_file_name, "r");

    for (int i = 0; i < FRAME_SIZE; ++i) {
        char current_byte;

        // read each byte and handle newline characters
        do {
            fread(&current_byte, 1, 1, process_file);
        } while (current_byte == '\n');  // Skip newline characters

        // store the byte in RAM
        manager->ram[frame_number * FRAME_SIZE + i] = current_byte;
    }

    fclose(process_file);

    // update the page table
    manager->page_table[page_number].valid = 1;
    manager->page_table[page_number].frame_number = frame_number;
}


void translate_logical_to_physical(MemoryManager *manager, char *process_file_name, int logical_address) {
    int page_number = logical_address / FRAME_SIZE;
    int offset = logical_address % FRAME_SIZE;

    if (!manager->page_table[page_number].valid) {
        handle_page_fault(manager, process_file_name, logical_address);
    }

    int frame_number = manager->page_table[page_number].frame_number;
    int physical_address = (frame_number * FRAME_SIZE) + offset;
    char data = manager->ram[physical_address];

    // increment the counter for the accessed page
    manager->page_table[page_number].counter++;

    // output results
    printf("%d\t\t%d\t\t\t%c\n", logical_address, physical_address, data);
}

int main(int argc, char ** argv) {
    if(argc != 3) {
        printf("Invalid no of args!\n");
        exit(1);
    }

    char *processes = argv[2],
         *addresses = argv[1];

    MemoryManager manager;
    manager.page_faults = 0;

    for (int i = 0; i < PAGE_TABLE_SIZE; ++i) {
        manager.page_table[i].counter = 0;
        manager.page_table[i].valid = 0;
        manager.page_table[i].frame_number = 0;
    }

    FILE *addresses_file = fopen(addresses, "r");
    if (!addresses_file) {
        fprintf(stderr, "Error: Could not open %s\n", "addresses.txt");
        return EXIT_FAILURE;
    }

    printf("Logical Address\tPhysical Address\tData\n");

    int logical_address = 0;

    while (fscanf(addresses_file, "%d", &logical_address) == 1) {
        translate_logical_to_physical(&manager, processes, logical_address);
    }

    fclose(addresses_file);

    // output statistics
    printf("\nPage-fault rate: %.2f%%\n", (float)manager.page_faults / 300 * 100);

    return EXIT_SUCCESS;
}
