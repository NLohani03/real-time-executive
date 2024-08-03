/*
 * k_task.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: any C functions you write must go into a corresponding c file that you create in the
 * Core->Src folder
 */

#ifndef INC_K_TASK_H_
#define INC_K_TASK_H_

#include "tcb.h"
#include "common.h"

#define SHPR2 *(uint32_t*)0xE000ED1C //for setting SVC priority, bits 31-24
#define SHPR3 *(uint32_t*)0xE000ED20 //PendSV is bits 23-16

void null_task(void*args);
/**
 * Create a new task and register it with the RTX if possible. Once created, each task is given a
 * unique task id (TID). A TID is an integer between 0 and N-1, where N is the maximum number of
 * tasks that the kernel supports (including the null task), and is decided by the MAX_TASKS macro
 * defined in the common.h file.  It is expected that the application code sets up the relevant TCB
 * fields before calling this function. Before returning, a successful call to osCreateTask updates
 * the relevant  kernel-level data structures with the information stored in the given TCB, and
 * updates that TCB with the unique TID that has been allocated to that  task. Please note that the
 * caller of this function never blocks, but it could be pre-empted.
 *
 * @returns This function returns RTX_OK on success and RTX_ERR on failure. Failure happens when a
 * new task cannot be created because of  invalid input(s) or the state of the RTX. For example, the
 * function returns RTX_ERR when the number of tasks has reached is maximum or the stack size in the
 * TCB is too small (i.e., less than STACK_SIZE). There are other failure modes as well that will be
 * addressed in  future projects (for instance, if no memory exists to allocate a new stack).
 */
int osCreateTask(TCB* task);

/**
 * This function receives two arguments: a TID and a pointer to a TCB. The function will retrieve
 * the information from the TCB of the task with id TID, and fill the TCB pointed to by task_copy
 * with all of its fields, if a task with the given TID exists. The automatic evaluation code will
 * use this function to copy TCB data into a new TCB so that any further manipulations will not
 * affect the existing TCBs in the OS. However, it should be noted that the application code is
 * expected to ensure that the TCB pointed to by task exists and can be safely modified.
 *
 * @returns This function returns RTX_OK if a task with the given TID exists, and RTX_ERR otherwise.
 */
int osTaskInfo(task_t TID, TCB* task_copy);

/**
 * This function, when called by a running task, immediately causes the task to exit and the
 * scheduler to be called, much in the same way that osYield works. However, when this function is
 * called the calling task is removed from the scheduler and any resources it was using are returned
 * to the operating system as per your design for future use, including re-using the same TID and/or
 * stack for subsequent tasks. For this deliverable this may be as simple as setting its status to
 * DORMANT. However, for subsequent deliverables you will need to carefully consider things like
 * memory allocation.
 *
 * @returns This function returns a value of RTX_OK if it was called by a running task. Otherwise,
 * it should return RTX_ERR and do nothing.
 */
int osTaskExit(void);

int osCreateDeadlineTask(int deadline, int s_size, TCB* task);

#endif /* INC_K_TASK_H_ */
