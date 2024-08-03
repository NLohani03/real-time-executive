#include "queue.h"
#include <stdio.h> 

int queue_full(struct Queue* queue){
    if(queue->size == MAX_TASKS-1){
        return true;
    }
    else{
        return false;
    }
}

int queue_empty(struct Queue* queue){
    if(queue->size == 0){
        return true;
    }
    else{
        return false;
    }
}

void queue_init(struct Queue* queue) {
    queue->first = 0;
    queue->last = 0;
    queue->size = 0;
}

int push(struct Queue* queue, TCB tcb){
    //  __disable_irq();
    // printf("This is a test \r\n");
    // printf("This is a different test %d\r\n", tcb->deadline);
    // __enable_irq();
    // TO DO: WE ARE PUSHING INTO 1 FIRST, NOT 0
    if(queue_full(queue)){
        return false;
    }
    queue->tcb_array[queue->last] = tcb;
    queue->last = (queue->last + 1) % (MAX_TASKS-1);        // This is 1 on the first item, when it should be 0
    queue->size++;    
    return true;
}

TCB pop(struct Queue* queue){
    if(queue_empty(queue)){
        // returns a TCB with the state -1
        TCB failed;
        failed.state = -1;
        return failed;
    }
    TCB tcb = queue->tcb_array[queue->first];
    queue->first = (queue->first + 1) % (MAX_TASKS-1);
    queue->size--;
    return tcb;
}




void createQueue(struct EDF_Queue* queue) {
    queue->first = NULL;
    queue->size = 0;
}

int pushToEDFQueue(struct EDF_Queue* queue, TCB *tcb) {
    if (queue->size > 16) {
        return false;
    }
    TCB* currNode = queue->first;
    int added = false;

    if (currNode == NULL) { //this is the first tcb being added
        tcb->next = NULL;
        queue->first = tcb;
        //printf("inserted at front of list\r\n", tcb);
    } else if (tcb->deadline < queue->first->deadline) {  // tcb needs to be added to the front because it has the earliest deadline out of all the tcbs
        tcb->next = queue->first;
        queue->first = tcb;
        //printf("inserted at front of list\r\n", tcb);
    } else {
        while (currNode != NULL && added == false) {  // currNode will only be NULL if we're adding the first Node
        	if (currNode->next == NULL) { // reached the end of the list (add to the end of the list)
				tcb->next = NULL;
				currNode->next = tcb;
				added = true;
                //printf("inserted at end of list\r\n", tcb);
			} else if (currNode->next->deadline > tcb->deadline) { // put it somewhere in the middle
                tcb->next = currNode->next;
                currNode->next = tcb;
                added = true;
                //printf("inserted in middle of list\r\n");
            }
            currNode = currNode->next;
        }
    }
    // queue->last = (queue->last + 1) % (MAX_TASKS-1);        // This is 1 on the first item, when it should be 0
    queue->size++;    
    return true;
}


int updatePositionInEDFQueue(struct EDF_Queue* queue, TCB* node) {
    if (queue->size <= 0) {
        return false;
    }
    TCB* current = queue->first;
    if (current == node) {
        queue->first = current->next;
        queue->size--;
        pushToEDFQueue(queue, node);
    } else {
        while (current != NULL) {
            if (current->next == node) {
                current->next = node->next;
                node->next = NULL;
                queue->size--;
                pushToEDFQueue(queue, node);
                break;
            }
            current = current->next;
        }
    }
    if (current == NULL) { // node wasn't found in the queue
        return false;
    }
    return true;
}

int removeFromEDFQueue(struct EDF_Queue* queue, TCB* node) {
    if (queue->size <= 0) {
        return false;
    }
    TCB* current = queue->first;
    if (current == node) {
        queue->first = current->next;
        queue->size--;
    } else {
        while (current != NULL) {
            if (current->next == node) {
                current->next = node->next;
                node->next = NULL;
                queue->size--;
                break;
            }
            current = current->next;
        }
    }
    if (current == NULL) { // node wasn't found in the queue
        return false;
    }
    return true;
}


int removeFirstItemFromEDFQueue(struct EDF_Queue* queue) {
    if (queue->size <= 0) {
        return false;
    }
    TCB* firstTCB = queue->first;
    queue->first = queue->first->next; 
    queue->size--;
    return true;
}

extern TCB nullTask;
TCB* get_non_sleeping_task(struct EDF_Queue* queue) {
    TCB* current = queue->first;
    while (current != NULL) {
        if (current->state != SLEEPING && current->tid != TID_NULL) {
            return current;
        }
        current = current->next;
    }
    return &nullTask; //all tasks are sleeping, return the null task
}

TCB* get_task_from_tid(struct EDF_Queue* queue, task_t TID) {
    TCB* current = queue->first;
    while (current != NULL) {
        if (current->tid == TID) {
            return current;
        }
        current = current->next;
    }
    return NULL; // task TID doesn't exist
}