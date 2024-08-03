#include "common.h"
#include "k_task.h"
#include "queue.h"
#include "main.h"
#include <stdio.h> 

#define SVC(code) asm volatile ("SVC %[immediate]"::[immediate] "I" (code))
int8_t kernel_started = false;
TCB currently_running_task = {0};
TCB *currently_running_task_edf_queue;
const uint32_t* msp_init_val;
const uint32_t* psp_init_val;
int8_t kernel_initialized = false;
extern struct Queue tcbs_queue;
extern struct EDF_Queue queue_of_tcbs;
uint64_t currentTimeCounter;
TCB nullTask;

int yield_from_tick=0;

void osKernelInit(void) {
    
    SHPR3 |= 0xFFU << 24; //Set the priority of SysTick to be the weakest
    SHPR3 |= 0xFEU << 16; //shift the constant 0xFE 16 bits to set PendSV priority
    SHPR2 |= 0xFDU << 24; //set the priority of SVC higher than PendSV

    queue_init(&tcbs_queue);

    msp_init_val = *(uint32_t**) 0x0;
    psp_init_val = msp_init_val - 0x400; // 0x400 words

    createQueue(&queue_of_tcbs);
    currentTimeCounter = 1;

    kernel_initialized = true; 
    return;
}

void switch_task(uint32_t* stack_pointer) {

    stack_pointer = __get_PSP(); 

    // Save the current stack info in the TCB and Task queue
    if (currently_running_task_edf_queue != NULL) {
        currently_running_task_edf_queue->current_stack_addr = stack_pointer;
    }

	currently_running_task_edf_queue = get_non_sleeping_task(&queue_of_tcbs);

    stack_pointer = currently_running_task_edf_queue->current_stack_addr;

    // If this task has not yet been started, set its registers to default values
   if ((!currently_running_task_edf_queue->been_initialized) && currently_running_task_edf_queue->state != RUNNING) {
        currently_running_task_edf_queue->been_initialized = 1;
        *(--stack_pointer) = 1<<24; //this is xPSR, setting the chip to Thumb mode
        *(--stack_pointer) = currently_running_task_edf_queue->ptask; // this is PC
		for (int i = 0; i < 14; ++i) {
		    *(--stack_pointer) = 0; // Fill the remaining registers
		}
        currently_running_task_edf_queue->state = RUNNING;
    }
    
   	currently_running_task_edf_queue->state = RUNNING;
    __set_PSP(stack_pointer);  // set the stack pointer to the correct address
}

int osKernelStart(void) {
    if (kernel_started || !kernel_initialized) { 
        return RTX_ERR;
    }
    currentTimeCounter = 1;
    __enable_irq();
    kernel_started = true;
//    if(queue_empty(&queue_of_tcbs)) {
        nullTask.ptask = &null_task;
        //nullTask.stack_size = 0x400;
        // nullTask.timeslice_initial = 1; 
        // nullTask.timeslice_length = 1; 
        //nullTask.deadline = (uint32_t) - 1;
        osCreateDeadlineTask(2147483647, 0x400, &nullTask);
//    }
    SVC(17);
}


void osYield(void) {  

    // //check if its called by the systick or the regular osyield 
    // if (yield_from_tick) {
    //     yield_from_tick = 0; //reset this value 

    //     //call pendsv directly from here 
    //     //do all the svc bs 
    //     __enable_irq(); 

    //     SVC_temp(); 
    // } else { 
    //     //currently_running_task_edf_queue->timeslice_length = currently_running_task_edf_queue->timeslice_initial; 
    //     SVC(17);
    // }
	__disable_irq();
	currently_running_task_edf_queue->deadline = currentTimeCounter + currently_running_task_edf_queue->initialDeadline;
    updatePositionInEDFQueue(&queue_of_tcbs, currently_running_task_edf_queue);
    __enable_irq();
    SVC(17);
    return;
}

void osSleep(int timeInMs) {
	__disable_irq();
    currently_running_task_edf_queue->state = SLEEPING;
    currently_running_task_edf_queue->deadline = timeInMs + currentTimeCounter;
    updatePositionInEDFQueue(&queue_of_tcbs, currently_running_task_edf_queue);
    __enable_irq();
    SVC(17);
    
}

void osPeriodYield() {
    currently_running_task_edf_queue->state = SLEEPING;
    SVC(17);
}



int osSetDeadline(int deadline, task_t TID) {
    if(deadline <= 0){
        return RTX_ERR;
    }
    // disable interrupts
    __disable_irq();

    // find task_t
    TCB* task = get_task_from_tid(&queue_of_tcbs, TID);
    if(task == NULL || task->state != READY){
        __enable_irq();
        return RTX_ERR;
    }

    // set the deadline of that task
    //printf("updated deadline from %d ", task->deadline);
     task->deadline = deadline + currentTimeCounter;
    //printf("to %d \r\n", task->deadline);

    //printf("updated initial deadline from %d ", task->initialDeadline);
    task->initialDeadline = deadline;
    //printf("to %d \r\n", task->initialDeadline);

    // enable interrupts
    __enable_irq();

     // update the position
    updatePositionInEDFQueue(&queue_of_tcbs, task);
    SVC(17);
    return RTX_OK;
}
