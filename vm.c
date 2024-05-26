#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#define MEMORY_SIZE 128 * 256
#define PAGE_SIZE 256
#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 128

typedef struct TLB_entry {
    int page_number;
    int frame_number;
} TLB_entry;

typedef struct Page_table_entry {
    int page_number;
    int frame_number;
    struct Page_table_entry* next;
} Page_table_entry;

TLB_entry tlb[TLB_SIZE];
Page_table_entry* page_table = NULL;

signed char memory[MEMORY_SIZE];
int next_free_frame = 0;
int tlb_size = 0;

int page_fault_count = 0;
int tlb_hit_count = 0;
int total_addresses = 0;

int tlb_index = 0;

FILE *backing_store;

typedef struct FrameQueueNode {
    int frame_number;
    struct FrameQueueNode* next;
} FrameQueueNode;

FrameQueueNode* frame_queue_front = NULL;
FrameQueueNode* frame_queue_rear = NULL;
int frame_queue_size = 0;

void initialize() {
    for (int i = 0; i < TLB_SIZE; i++) {
        tlb[i].page_number = -1; // Inicializa as entradas do TLB como inválidas
    }
    page_table = NULL;
    memset(memory, 0, MEMORY_SIZE);
}

int search_tlb(int page_number) {
    for (int i = 0; i < TLB_SIZE; i++) {
        if (tlb[i].page_number == page_number) {
            tlb_hit_count++;
            return i;
        }
    }
    return -1;
}

void add_to_tlb(int page_number, int frame_number) {
    tlb[tlb_index].page_number = page_number;
    tlb[tlb_index].frame_number = frame_number;
    tlb_index = (tlb_index + 1) % TLB_SIZE;
}

Page_table_entry* search_page_table(int page_number) {
    Page_table_entry* current = page_table;
    while (current != NULL) {
        if (current->page_number == page_number) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

void enqueue_frame(int frame_number) {
    FrameQueueNode* new_node = (FrameQueueNode*)malloc(sizeof(FrameQueueNode));
    new_node->frame_number = frame_number;
    new_node->next = NULL;
    if (frame_queue_rear == NULL) {
        frame_queue_front = frame_queue_rear = new_node;
    } else {
        frame_queue_rear->next = new_node;
        frame_queue_rear = new_node;
    }
    frame_queue_size++;
}

int dequeue_frame() {
    if (frame_queue_front == NULL) {
        return -1; // Fila está vazia
    }
    int frame_number = frame_queue_front->frame_number;
    FrameQueueNode* temp = frame_queue_front;
    frame_queue_front = frame_queue_front->next;
    if (frame_queue_front == NULL) {
        frame_queue_rear = NULL;
    }
    free(temp);
    frame_queue_size--;
    return frame_number;
}

int add_to_page_table(int page_number) {
    int frame_number;
    if (next_free_frame < PAGE_TABLE_SIZE) {
        frame_number = next_free_frame++;
    } else {
        frame_number = dequeue_frame();
        Page_table_entry* current = page_table;
        Page_table_entry* prev = NULL;
        while (current != NULL) {
            if (current->frame_number == frame_number) {
                if (prev == NULL) {
                    page_table = current->next;
                } else {
                    prev->next = current->next;
                }
                free(current);
                break;
            }
            prev = current;
            current = current->next;
        }
    }

    Page_table_entry* new_entry = (Page_table_entry*)malloc(sizeof(Page_table_entry));
    new_entry->page_number = page_number;
    new_entry->frame_number = frame_number;
    new_entry->next = page_table;
    page_table = new_entry;

    page_fault_count++;
    enqueue_frame(frame_number);
    return new_entry->frame_number;
}

int main(int argc, char *argv[]) {

    FILE *addresses_file = fopen(argv[1], "r");

    backing_store = fopen("BACKING_STORE.bin", "rb");

    FILE *output_file = fopen("correct.txt", "w");
    
    initialize();

    char address[10];
    while (fgets(address, 10, addresses_file) != NULL) {
        int logical_address = atoi(address);
        int page_number = (logical_address >> 8) & 0xFF;
        int offset = logical_address & 0xFF;

        int tlb_entry_index = search_tlb(page_number);
        int frame_number;
        if (tlb_entry_index != -1) {
            frame_number = tlb[tlb_entry_index].frame_number;
        } else {
            Page_table_entry* page_table_entry = search_page_table(page_number);
            if (page_table_entry != NULL) {
                frame_number = page_table_entry->frame_number;
            } else {
                frame_number = add_to_page_table(page_number);

                fseek(backing_store, page_number * PAGE_SIZE, SEEK_SET);
                fread(&memory[frame_number * PAGE_SIZE], sizeof(char), PAGE_SIZE, backing_store);
            }
            add_to_tlb(page_number, frame_number);
            tlb_entry_index = (tlb_index - 1 + TLB_SIZE) % TLB_SIZE;
        }

        int physical_address = frame_number * PAGE_SIZE + offset;
        signed char value = memory[physical_address];

        fprintf(output_file, "Virtual address: %d TLB: %d Physical address: %d Value: %d \n",
                logical_address, tlb_entry_index, physical_address, value);
        total_addresses++;
    }

    fprintf(output_file, "Number of Translated Addresses = %d\n", total_addresses);
    fprintf(output_file, "Page Faults = %d\n", page_fault_count);
    fprintf(output_file, "Page Fault Rate = %.3f\n", (float) page_fault_count / total_addresses);
    fprintf(output_file, "TLB Hits = %d\n", tlb_hit_count);
    fprintf(output_file, "TLB Hit Rate = %.3f\n", (float) tlb_hit_count / total_addresses);

    fclose(addresses_file);
    fclose(backing_store);
    fclose(output_file);

    return 0;
}