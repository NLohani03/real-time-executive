#include "k_mem.h"

#include "common.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
// #include "stm32f4xx_hal.h"

extern int8_t kernel_initialized;
extern TCB currently_running_task;
extern TCB *currently_running_task_edf_queue;
int8_t k_mem_initialized = false;

extern uint32_t _img_end;
extern uint32_t _estack;
extern uint32_t _Min_Stack_Size;

uint32_t* heap_end;
uint32_t* heap_start;

struct Metadata* freelist_start;
struct Metadata* freelist_end;
// struct Metadata* alloclist_start;
// struct Metadata* alloclist_end;

int k_mem_init() {

	if(!kernel_initialized || k_mem_initialized) {
		return RTX_ERR;
	}

	heap_end = (uint32_t)&_estack - (uint32_t)&_Min_Stack_Size;
	heap_start = &_img_end;
	
	freelist_start = (struct Metadata*)heap_start;
	freelist_end = (struct Metadata*)freelist_start;

	(*freelist_start).next = NULL;
	(*freelist_start).tid = TID_NULL;
	(*freelist_start).block_size = ((uint32_t)heap_end - (uint32_t)heap_start);

	k_mem_initialized = true;

	return RTX_OK;
}


void* k_mem_alloc(size_t size) {
	// __disable_irq();
	// 	printf("k_mem_initialized BOOL: %d\r\n", k_mem_initialized);
    // __enable_irq();
	//look for free space in the empty linked list

	//add node in the deallocated list
	if (!k_mem_initialized) { return (void*)NULL; }
	if(size == 0) { return (void*)NULL; }
    if (size%4) {   // changing size to fill 4 byte chunks (allocations are 4-byte aligned)
        size = size+4-size%4;
    }

	// printf("trying to allocate memory of size %lu after initialization and size is not 0\r\n", size); 

	//block to start traversing from and the prev block t
	struct Metadata* current_block = freelist_start; 
	struct Metadata* prev_block = NULL; 
	
	//traverse through the nodes and find a block that fits the size 

	/**
	 * The next pointer of the current block points to the address of the next block
	 * Subtracting the current block address from the next pointer address will give us the size of the block between the two
	 * */
	while(current_block != NULL) {
		size_t block_size = current_block->block_size; 
		// __disable_irq();
		// printf("block size, allocation: %d\r\n", block_size);
		// __enable_irq();
		if (block_size >= size) { 
			//block is big enough, allocate the size + size of metadata here 
			struct Metadata* allocated_block = current_block; 
			allocated_block->block_size = size;
			// printf("updated allocated_block size to: %d\r\n", allocated_block->block_size);
			long new_block_size = block_size - size - sizeof(struct Metadata);

			//splitting the block size 
			if (new_block_size > 0) { 
				//need to cast (char*) apparently to make sure we can add by bytes and not size of Metadata
				if (new_block_size <= sizeof(struct Metadata)){
					allocated_block->block_size = allocated_block->block_size + new_block_size; 
				} else { 
					struct Metadata* new_free_block = (struct Metadata*)((char*)current_block + size + sizeof(struct Metadata)); 
					new_free_block->next = current_block->next; 
					new_free_block->tid = TID_NULL; 
					new_free_block->block_size = new_block_size;
					current_block->next = new_free_block; 
				}
			}

			//removing from the allocated list 
			if (prev_block != NULL) { //list is not empty
				prev_block->next = current_block->next; 
			} else { 
				freelist_start = current_block->next; //list only has one thing in it 
			}

			//add allocated block to the allocated list
			allocated_block->tid = currently_running_task_edf_queue->tid; //the task that requested malloc
			// printf("allocated block tid %u \r\n", allocated_block->tid);
			allocated_block->flag = true;
			return (void*)((void*)allocated_block + sizeof(struct Metadata));
			//unsure abt the sizeof(Metadata), do we want the pointer to pointer to the metadata part too or just the allocated block?
		}
		prev_block = current_block; 
		current_block = current_block->next; 
	} 
	return (void*)NULL;
}

int k_mem_dealloc(void* ptr) {
    heap_start = &_img_end;
	

	// go to ptr
	// go back sizeof(struct Metadata) to get the start of the metadata
    struct Metadata* startOfMetadata = (struct Metadata*) (ptr - sizeof(struct Metadata));
	// use metadata to get size of block
    size_t blockSize = startOfMetadata->block_size;
    // printf("startOfMetadata: %p\r\n", (void*)startOfMetadata);
	// iterate through freelist (starting at freelist_start) to find this memory location, when the address of current_block->next greater address of ptr
	struct Metadata* after = freelist_start;
	struct Metadata* prev = NULL;

    while (after->next != NULL && (void*)(after) <= (void*)startOfMetadata) {
		prev = after;
        after = after->next;
    }

    if ((void*)(after) <= (void*)startOfMetadata) {
        // already deallocated block
        return RTX_ERR;
    }

    if (startOfMetadata->flag == true) {
        startOfMetadata->flag = false;
    } else {
        return RTX_ERR;
    }

    if (startOfMetadata->tid != currently_running_task.tid && currently_running_task.tid != TID_NULL) {
        return RTX_ERR;
    }
    
    // struct Metadata* current = &next-blockSize-sizeof(struct Metadata);
    // check if current_block+size is ptr (there's no other blocks in between)
	// coalecense if so (change size of current_block->size)
	void* end_address = (void*)startOfMetadata+blockSize+sizeof(struct Metadata);
	void* prev_end_address = (void*)(prev)+prev->block_size+sizeof(struct Metadata);

    if (prev_end_address == (void*)startOfMetadata && end_address == (void*)after) {
        prev->block_size=prev->block_size+blockSize+sizeof(struct Metadata) + sizeof(struct Metadata)+after->block_size;
        startOfMetadata = prev;
        prev->next = after->next;
    } else {
        if (prev_end_address == (void*)startOfMetadata) {
            // preform coalacence
            prev->block_size=prev->block_size+blockSize+sizeof(struct Metadata);
            startOfMetadata = prev;
        }
        if (end_address == (void*)after) {  // coalanence with block after
            // preform coalacence
            startOfMetadata->block_size = blockSize + sizeof(struct Metadata) + (after->block_size);
            if(prev == NULL){
				freelist_start = startOfMetadata;
			}
			else{
				prev->next = startOfMetadata;
			}
			startOfMetadata->next = after->next;
        }
    }
	// check if ptr+size is current_block->next
    // coalecense (change size of ptr->size, change current_block->next to ptr, change ptr->next to current_block->next->next)
		
	// if no coalecense, change current_block->next to ptr, ptr->next to current_block->next
    if (end_address != (void*)after && prev_end_address != (void*)startOfMetadata) {
		if(prev == NULL){
			freelist_start = startOfMetadata;
		}
		else{
			prev->next = startOfMetadata;
		}
        startOfMetadata->next = after;
    }
	return RTX_OK;
}

int k_mem_count_extfrag(size_t size) {
	if (!k_mem_initialized) { return 0; }
	//iterate through the free list and count the number of empty spaces less than size
	int extfrag_count = 0; 
	struct Metadata* current_block = freelist_start;

	while (current_block != NULL) {
		if (current_block->block_size < size) { 
			extfrag_count++; 
		}
		current_block = current_block->next; 
	}
	return extfrag_count;
}

