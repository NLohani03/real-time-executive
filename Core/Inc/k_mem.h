/*
 * k_mem.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: any C functions you write must go into a corresponding c file that you create in the
 * Core->Src folder
 */

#ifndef INC_K_MEM_H_
#define INC_K_MEM_H_

#include <stddef.h>
#include "k_task.h"

struct Metadata {
    task_t tid; // TID of the task that allocated this memory block
	// next implicitly stores the size of the block that was allocated
	struct Metadata* next; // Pointer to the next free or allocated memory block
	size_t block_size;
    char flag;
};

/**
 * Initializes the RTX’s memory manager. This should include locating the heap and writing the
 * initial metadata required to start searching for free regions. As the manager allocates and
 * deallocates memory (see k_mem_alloc and k_mem_dealloc), the memory will be partitioned into free
 * and allocated regions. Your metadata data structures will be used to keep track of all of these
 * regions.
 * 
 * @returns RTX_OK on success and RTX_ERR on failure, which happens if this function is called more
 * than once or if it is called before the kernel itself is initialized.
 */
int k_mem_init();

/**
 * Allocates size bytes according to the First Fit algorithm, and returns a pointer to the start of
 * the usable memory in the block (that is, after the metadata). The first-fit iteration should
 * start from the beginning of the free memory region. The k_mem_init function must be called before
 * this function is called, otherwise this function must return NULL. The size argument is the
 * number of bytes requested. Any metadata must be allocated in addition to size bytes. If size is
 * 0, this function returns NULL. The allocated memory is not initialized. Memory requests may be of
 * any size. By default this function assigns the memory region requested to the currently running
 * task, or to the kernel if no tasks are running. This ownership information must be stored in the
 * metadata, typically by storing the task ID of the task that allocated the memory, or some obvious
 * value if the kernel allocated the memory.
 * 
 * @returns a pointer to allocated memory or NULL if the request fails. Failure happens if the RTX
 * cannot allocate the requested memory, either for a reason stated above or because there are no
 * blocks of suitable size available
 */
void* k_mem_alloc(size_t size);

/**
 * This function frees the memory pointed to by ptr as long as the currently running task is the
 * owner of that block and as long as the memory pointed to by ptr is in fact allocated (see below).
 * Otherwise, or if this memory is already free, this function should return RTX_ERR. If ptr is
 * NULL, no action is performed. If the newly freed memory region is adjacent to other free memory
 * regions, they have to be merged immediately and the combined region is re-integrated into the
 * free memory under management. This function does not clear the content of the newly freed region.
 * To determine if a memory region has been allocated, this function should examine the metadata
 * immediately before the address ptr. You therefore must record in your metadata some way of
 * determining this information. You may assume that, during testing, we will not do anything
 * pathological. For example, we will not allocate 1000 bytes, then write valid metadata throughout
 * that memory, and then attempt to free a random location. You may not assume that we will not
 * attempt to free a random location in memory, though.
 * 
 * @returns RTX_OK on success and RTX_ERR on failure. Failure happens when the RTX cannot
 * successfully free the memory region for some reason (some of which are explained above).
 */
int k_mem_dealloc(void* ptr);

/**
 * This function counts the number of free memory regions that are strictly less than size bytes.
 * The space your memory management data structures occupy inside of each free region is considered
 * to be free in this context, but not each allocated region. For example, assume that the memory
 * status is as follows: The grey regions are occupied by the metadata, the white indicates free
 * space that can be allocated, and the blue regions indicate allocated memory regions. Calling
 * k_mem_count_extfrag with 12, 42, and 43 as inputs should return 0, 2, and 3, respectively. If
 * memory has not yet been initialized, return 0, as there are no free blocks of any size available.
 */
int k_mem_count_extfrag(size_t size);

#endif /* INC_K_MEM_H_ */
