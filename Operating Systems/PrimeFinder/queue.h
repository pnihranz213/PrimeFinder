#ifndef QUEUE_H
#define QUEUE_H

#include <stdio.h>
#include <stdlib.h>

typedef struct Process {
    int id, pid;
    int burst;
    struct Process* next;
} Process;

typedef struct Queue {
    Process* front;
    Process* rear;
} Queue;


void enqueue(Queue* q, int id, int burstTime, int pid) ;
Process* dequeue(Queue* q) ;
int isEmpty(Queue* q) ;
Queue* createQueue() ;
void destroyQueue(Queue* q) ;

#endif // QUEUE_H
