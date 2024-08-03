#ifndef INC_TCB_H_
#define INC_TCB_H_

#include "common.h"

typedef struct task_control_block {
    void (*ptask)(void* args);  // entry address
    uint32_t stack_high;        // start starting address (high address)
    task_t tid;                 // task ID
    uint8_t state;              // task's state
    uint16_t stack_size;        // stack size. Must be a multiple of 8
    uint32_t* current_stack_addr;
    // uint8_t timeslice_length;      //time remaining before this task must be switched out
    // uint8_t timeslice_initial;  //initial timeslice given for each task 
    uint32_t initialDeadline;    // first deadline - constant - does not ever change because we are using it to know how much to change the deadline by every time
    uint32_t deadline;   // tast's current deadline - will change every time the deadline has arrived
    struct task_control_block* next;
    uint32_t* stack_pointer;        // Pointer to the allocated stack
    uint8_t been_initialized; 
} TCB;

#include <stddef.h>
#endif