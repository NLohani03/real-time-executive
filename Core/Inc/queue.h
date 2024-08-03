/*
 * k_task.h
 *
 *  Created on: Jan 5, 2024
 *      Author: nexususer
 *
 *      NOTE: any C functions you write must go into a corresponding c file that you create in the
 * Core->Src folder
 */
#ifndef INC_QUEUE_H_
#define INC_QUEUE_H_

#include "tcb.h"
#include "k_mem.h"

struct Queue {
    int first, last, size;
    TCB tcb_array[MAX_TASKS];
};

typedef struct Node {
    TCB tcb;
    struct Node* next;
} Node;
typedef struct EDF_Queue {
    TCB* first;
    int size;
} EDF_Queue;

int queue_full(struct Queue* queue);
int queue_empty(struct Queue* queue);
void queue_init(struct Queue* queue);
int push(struct Queue* queue, TCB tcb);
TCB pop(struct Queue* queue);
void createQueue(struct EDF_Queue* queue);
int pushToEDFQueue(struct EDF_Queue* queue, TCB *tcb);
int removeFirstItemFromEDFQueue(struct EDF_Queue* queue);
int removeFromEDFQueue(struct EDF_Queue* queue, TCB* node);
TCB* get_non_sleeping_task(struct EDF_Queue* queue);
TCB* get_task_from_tid(struct EDF_Queue* queue, task_t TID);
int updatePositionInEDFQueue(struct EDF_Queue* queue, TCB* node);

#endif /* INC_K_TASK_H_ */
