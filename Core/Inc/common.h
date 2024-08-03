/*
 * common.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: If you feel that there are common
 *      C functions corresponding to this
 *      header, then any C functions you write must go into a corresponding c file that you create
 * in the Core->Src folder
 */

#ifndef INC_COMMON_H_
#define INC_COMMON_H_

#define TID_NULL 0      // predefined Task ID for the NULL task

#define STACK_SIZE 0x200  // min. size of each task’s stack
#define DORMANT 0         // state of terminated task
#define READY 1           // state of task that can be scheduled but is not running
#define RUNNING 2         // state of running task
#define SLEEPING 3

#define RTX_ERR -1
#define RTX_OK 0

#define MAX_TASKS 16      // maximum number of tasks in the system

#define true 1
#define false 0

#include <stdint.h>
typedef unsigned int task_t;

/**
 * This function initializes all global kernel-level data structures and other variables as required
 * by your design. This function must be called before any other RTX functions will work. Since this
 * function is used to set up your RTX, what it does and how it does it is largely up to your group.
 * It is essential that everyone in the group understands and agrees to how this function works
 * before attempting to write others.
 */
void osKernelInit(void);

/**
 * This function is called by application code after the kernel has been initialized to run the
 * first task. It is assumed that once the first task starts, subsequent tasks are run via various
 * methods including yielding and pre-emption. Therefore, if this function is called more than once,
 * or is called before the kernel is initialized, no action should be taken. Groups will likely use
 * this function to initialize the scheduling algorithm given how tasks are stored.
 *
 * @returns This function returns RTX_ERR if the kernel is not initialized or if the kernel is
 * already running. Otherwise, it will not return. The expected successful behaviour is for the
 * kernel to start running tasks immediately.
 */
int osKernelStart(void);

/**
 * This function immediately halts the execution of one task, saves it contexts, runs the scheduler,
 * and loads the context of the next task to run. The expected behaviour depends on whether the next
 * task has already been running, or if it is its first time running after the kernel has been
 * initialized. If the next task has been running already, it should resume exactly where it left
 * off. For example, if that task ran and then itself called osYield, the task should resume as
 * though it had returned from this function. If the next task has not yet run, it should start from
 * its entry point.
 *
 * Groups implementing this function must ensure that, at least, the following steps occur:
 * 1. The currently running task’s context is saved onto that task’s stack
 * 2. The scheduler is run and the next task is selected
 * 3. That thread’s context is loaded from that task’s stack
 * 4. The CPU resumes execution of the next task
 *
 * The context switch (saving and loading context) requires assembly code. Groups may elect to
 * handle certain operations, such as running the scheduler, by calling C functions from within
 * their assembly routines.
 */
void osYield(void);

/**
 * This function sets the deadline of the task with Id TID to the deadline stored in the
 * deadline variable. Any task can set the deadline of any other task. This function must block 
 * – that is, when it is called, the OS should not respond to any timer interrupts until it is 
 * completed. It is possible for this function to cause a pre-emptive context switch in the following 
 * scenario: presume task A is currently running and has a deadline of 5ms. task B is not running 
 * and has a deadline of 10ms. If task A calls this function to set task B’s deadline to 3ms, task 
 * B now has an earlier deadline and task A should be pre-empted
 * 
 * @returns this function should return RTX_OK if the deadline provided is strictly positive and a task
 * with the given TID exists and is ready to run. Otherwise it must return RTX_ERR
 */
int osSetDeadline(int deadline, task_t TID);

/**
 * This function immediately halts the execution of one task, saves it context, runs the scheduler,
 * and loads the context of the next task to run. In addition, this function removes this task from
 * scheduling until the provided sleep time has expired. Upon awakening, a task’s deadline is reset
 * to its initial value. If no tasks are available to run (that is, all tasks are sleeping), the
 * expected behaviour is for the NULL task to run until a task emerges from sleep. Groups may notice
 * that this function is very similar to osYield, and they would be correct. This function operates
 * the same as osYield except the task cannot be scheduled until the sleep time is up.
 */
void osSleep(int timeInMs);

/**
 * A periodic task will call this function instead of osYield when it has completed its current
 * instance. The task is not ready to execute until its current time period elapses, so it should
 * not be scheduled immediately even if the CPU is available. In practice, this function can have
 * the same behavior as osSleep, except that the sleep time is calculated automatically by the
 * Kernel to facilitate its periodicity.
 */
void osPeriodYield();


#endif /* INC_COMMON_H_ */
