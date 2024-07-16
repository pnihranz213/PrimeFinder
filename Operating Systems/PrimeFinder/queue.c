#include "queue.h"

void enqueue(Queue* q, int id, int burstTime, int pid) {
    Process* newProcess = (Process*)malloc(sizeof(Process));
    newProcess->id = id;
    newProcess->burst = burstTime;
    newProcess->pid = pid;
    newProcess->next = NULL;

    if (q->rear == NULL) {
        q->front = q->rear = newProcess;
        return;
    }

    q->rear->next = newProcess;
    q->rear = newProcess;
}

Process* dequeue(Queue* q) {
    if (q->front == NULL) {
        return NULL;
    }

    Process* frontProcess = q->front;
    q->front = q->front->next;

    if (q->front == NULL) {
        q->rear = NULL;
    }

    return frontProcess;
}

int isEmpty(Queue* q) {
    return (q->front == NULL);
}

Queue* createQueue() {
    Queue* q = (Queue*)malloc(sizeof(Queue));
    q->front = q->rear = NULL;
    return q;
}

void destroyQueue(Queue* q) {
    while (!isEmpty(q)) {
        Process* process = dequeue(q);
        free(process);
    }
    free(q);
}
