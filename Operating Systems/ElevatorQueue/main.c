#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <pthread.h>
#include <string.h>
#include <semaphore.h>
#include <stdarg.h>

#define MAX_FLOORS 100

enum sem_tp {
    wait,
    request
};

enum dir {
    up = 1,
    down = -1
};

struct activity {
    int n;
    int *f, *t;

    int mid;    // other info
};

sem_t printSemaphore;

void syncprintf(const char* format, ...) {
    va_list args;
    va_start(args, format);

    sem_wait(&printSemaphore);
    vprintf(format, args);
    sem_post(&printSemaphore);

    va_end(args);
}

// global vars
int numFloors = 10;

sem_t elevatorSemaphore;
sem_t floorSemaphores[MAX_FLOORS][2];
pthread_mutex_t mutex;

void initializeSemaphores() {
    sem_init(&elevatorSemaphore, 0, 0);
    sem_init(&printSemaphore, 0, 1);
    for (int i = 0; i < numFloors; i++) {
        sem_init(&floorSemaphores[i][0], 0, 0);
        sem_init(&floorSemaphores[i][1], 0, 0);
    }
    pthread_mutex_init(&mutex, NULL);
}

void cleanupSemaphores() {
    sem_destroy(&elevatorSemaphore);
    sem_destroy(&printSemaphore);
    for (int i = 0; i < numFloors; i++) {
        sem_destroy(&floorSemaphores[i][0]);
        sem_destroy(&floorSemaphores[i][1]);
    }
    pthread_mutex_destroy(&mutex);
}

void *elevatorThread(void *arg) {
    int maxWanderTime = *(int*)arg;

    int currentFloor = 0;
    static enum dir d = up;

    while (1) {
        int peopleWaiting;
        sem_getvalue(&floorSemaphores[currentFloor][request], &peopleWaiting);

        if (peopleWaiting > 0) {
            syncprintf("\tElevator: Opening the doors at %d\n", currentFloor);
            sleep(1);

            // notify persons waiting on the floor
            for (int i = 0; i < peopleWaiting; i++) {
                sem_post(&floorSemaphores[currentFloor][wait]);
                sem_wait(&floorSemaphores[currentFloor][request]);
            }
        }

        int elivator_requested = 0;
        if(currentFloor == 0) {
            sem_wait(&printSemaphore);
            printf("Printout From the Elevator:\n");
            printf("Number of people waiting at floor\n");
            printf("Floor Number\tNumber of People\n");
            for(int i=0; i<numFloors; ++i) {
                int peopleWaiting;
                sem_getvalue(&floorSemaphores[i][request], &peopleWaiting);
                printf("%d\t%d\n", i, peopleWaiting);
                elivator_requested |= peopleWaiting;
            }
            sem_post(&printSemaphore);

            if(!elivator_requested) {
                syncprintf("Waiting for max waiting time.\n");
                sleep(maxWanderTime);
                for(int i=0; i<numFloors; ++i) {
                    int peopleWaiting;
                    sem_getvalue(&floorSemaphores[i][request], &peopleWaiting);
                    syncprintf("%d\t%d\n", i, peopleWaiting);
                    elivator_requested |= peopleWaiting;
                }
            }
            if(!elivator_requested) {
                syncprintf("No one is waiting for the elevator.\n");
                break;
            }

            syncprintf("\tElevator: Heading to max Floor %d\n", numFloors);
            d = up;
        } else if(currentFloor == numFloors) {
            syncprintf("\tElevator: Heading to min Floor 0\n");
            d = down;
        }

        currentFloor += d;
    }

    syncprintf("Elevator Leaving The System\n");
    pthread_exit(NULL);
}

void *personThread(void *arg) {
    struct activity *a = (struct activity *)arg;
    int personID = a->mid;

    sem_wait(&printSemaphore);
    printf("Person %d: Floors To Visit ", a->mid);
    for(int i=0; i<a->n; ++i) {
        printf("%d", a->f[i]);
        if(i!=a->n-1)
            printf(",");
        else
            printf("\n");
    }

    printf("Person %d: Time To Spend ", a->mid);
    for(int i=0; i<a->n; ++i) {
        printf("%d", a->f[i]);
        if(i!=a->n-1)
            printf(",");
        else
            printf("\n");
    }
    sem_post(&printSemaphore);

    int nextDest = 0;

    syncprintf("Person Number %d: Waiting for elevator at floor 0\n", personID);
    while (nextDest < a->n) {
        int wanderTime = a->t[nextDest];

        syncprintf("Person Number %d: Taking elevator to floor %d\n", a->mid, a->f[nextDest]);
        sem_post(&floorSemaphores[a->f[nextDest]][request]);

        syncprintf("Person Number %d: Waiting to get off at floor %d\n", a->mid, a->f[nextDest]);
        sem_wait(&floorSemaphores[a->f[nextDest]][wait]);

        syncprintf("Person Number %d: Got off at floor %d\n", a->mid, a->f[nextDest]);

        syncprintf("Person Number %d: Wandering for %d seconds\n", a->mid, a->t[nextDest]);
        sleep(wanderTime);

        nextDest++;
    }

    syncprintf("Person Number %d:Leaving the System Goodbye!\n", a->mid);
    pthread_exit(NULL);
}

int main(int argc, char **argv) {
    int opt;
    int numPeople, maxWanderTime;

    while ((opt = getopt(argc, argv, "p:f:w:")) != -1) {
        switch (opt) {
        case 'p':
            numPeople = atoi(optarg);
            break;
        case 'w':
            maxWanderTime = atoi(optarg);
            break;
        case 'f':
            numFloors = atoi(optarg)-1;
            break;
        default:
            syncprintf("Usage: %s -p Npeople -w maxWanderTime -f Nfloors\n", argv[0]);
            exit(EXIT_FAILURE);
        }
    }

    struct activity* A[numPeople];
    for(int i=0; i<numPeople; ++i) {
        struct activity *a = (struct activity*)malloc(sizeof (struct activity));
        a->mid = i;

        scanf("%d", &a->n);
        a->f = (int*)malloc(sizeof (int) *a->n);
        a->t = (int*)malloc(sizeof (int) *a->n);

        for(int i=0; i<a->n; ++i) {
            scanf("%d", a->f+i);
            scanf("%d", a->t+i);
        }

        A[i] = a;
    }

    initializeSemaphores();

    pthread_t personThreads[numPeople];
    for (int i = 0; i < numPeople; i++) {
        pthread_create(&personThreads[i], NULL, personThread, (void *)A[i]);
    }

    sleep(2);

    pthread_t elevatorThreadID;
    pthread_create(&elevatorThreadID, NULL, elevatorThread, &maxWanderTime);


    for (int i = 0; i < numPeople; i++) {
        pthread_join(personThreads[i], NULL);
    }
    pthread_join(elevatorThreadID, NULL);

    cleanupSemaphores();

    return 0;
}
