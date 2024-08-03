#include <stdio.h> 
#include "k_task.h"
#include "queue.h"
#include "stm32f4xx_hal.h"

#define SVC(code) asm volatile ("SVC %[immediate]"::[immediate] "I" (code))
extern int8_t kernel_initialized;
task_t current_task_id = 0;
struct Queue tcbs_queue;   
extern uint64_t currentTimeCounter;
extern const uint32_t* msp_init_val;
extern const uint32_t* psp_init_val;
extern TCB currently_running_task;
extern TCB *currently_running_task_edf_queue;
static uint32_t sp_offset = 0;
int available_stack[MAX_TASKS-1] = {0};
int task_sizes[MAX_TASKS-1] = {0};
struct EDF_Queue queue_of_tcbs;

void null_task(void*args) { 
    while(1) {
        //printf("running the null task\r\n");
    }
}

int osCreateDeadlineTask(int deadline, int s_size, TCB* task) {

    if (!kernel_initialized) {
        return RTX_ERR; //failure kernel is uninitialized
    }
    if (task == NULL) {
        return RTX_ERR;
    }

    if (task->ptask == NULL) {
        return RTX_ERR; 
    }
    task->stack_size = s_size;
    task->deadline = deadline + currentTimeCounter;

    if (task->stack_size < STACK_SIZE || task->stack_size > 0x4400 ) { //does nothing for bad tasks cause the random value can be in between
        return RTX_ERR;
    }
    if(task->deadline <= 0) { 
        return RTX_ERR;
    }

    task->stack_pointer = (uint32_t*)k_mem_alloc(task->stack_size + sizeof(TCB));

    if (task->stack_pointer == NULL) {
        return RTX_ERR; 
    }
    task->stack_high = (uint32_t)task->stack_pointer + task->stack_size + sizeof(TCB);
    task->tid = ++current_task_id;
    if(task->ptask == null_task) { 
        task->tid = TID_NULL;
    }
    task->state = READY;
    task->current_stack_addr = task->stack_high; 
    task->initialDeadline = deadline;
    task->next = NULL;
    task->been_initialized = 0; 

    TCB* internal_task = (TCB*)task->stack_pointer;
    *internal_task = *task;
    __disable_irq();
    if (!pushToEDFQueue(&queue_of_tcbs, internal_task)) {
        k_mem_dealloc((void*)task->stack_pointer); 
        current_task_id--; 
        __enable_irq();
        return RTX_ERR;
    }
    __enable_irq();
    return RTX_OK;

}

int osCreateTask(TCB* task) {
    if (!kernel_initialized) {
        return RTX_ERR; //failure kernel is uninitialized
    }
    if (task == NULL) {
        return RTX_ERR;
    }

    if (task->ptask == NULL) {
        return RTX_ERR; 
    }
    
    if (task->stack_size < STACK_SIZE || task->stack_size > 0x4400 ) { //does nothing for bad tasks cause the random value can be in between
        return RTX_ERR;
    }

    task->stack_pointer = (uint32_t*)k_mem_alloc(task->stack_size + sizeof(TCB));
    if (task->stack_pointer == NULL) {
        return RTX_ERR; 
    }
    task->stack_high = (uint32_t)task->stack_pointer + task->stack_size + sizeof(TCB);


    task->tid = ++current_task_id;
    if(task->ptask == null_task) {
        task->tid = TID_NULL;
    }
    task->state = READY;
    task->current_stack_addr = task->stack_high; 
    task->deadline = 5 + currentTimeCounter;
    task->initialDeadline = 5;
    task->next = NULL;
    task->been_initialized = 0; 

    TCB* internal_task = (TCB*)task->stack_pointer;
    *internal_task = *task;

    __disable_irq();
    if (!pushToEDFQueue(&queue_of_tcbs, internal_task)) {
        k_mem_dealloc((void*)task->stack_pointer); 
        current_task_id--; 
        __enable_irq();
        return RTX_ERR;
    }
    __enable_irq();
    return RTX_OK;
}


int osTaskInfo(task_t TID, TCB* task_copy) {
    if (currently_running_task_edf_queue->tid == TID) {
        *task_copy = *currently_running_task_edf_queue;
        return RTX_OK;
    }

    TCB* current = queue_of_tcbs.first;
    while (current != NULL) {
        if (current->tid == TID && current->state != DORMANT) {
            *task_copy = *current;
            return RTX_OK;
        }
        current = current->next;
    }

    return RTX_ERR;
}

int osTaskExit(void) {
    __disable_irq();
    if(currently_running_task_edf_queue->tid == TID_NULL){
        __enable_irq();
        return RTX_ERR;
    }
    currently_running_task_edf_queue->state = DORMANT;
    removeFromEDFQueue(&queue_of_tcbs, currently_running_task_edf_queue);
    k_mem_dealloc((void*)currently_running_task_edf_queue->stack_pointer);
    __enable_irq();
    SVC(17);
    return RTX_OK;
}

