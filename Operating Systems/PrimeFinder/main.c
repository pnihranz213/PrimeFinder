#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <time.h>
#include "queue.h"

Queue* queue1;   // First queue
Queue* queue2;   // Second queue

Process *current_process = NULL;
int time_slice;
int time_now = 0;
int remaining_time_slice = 0;
int finished_scheduling = 0;

void schedule_process2();

void handle_SIGALRM(int) {
    if(current_process == NULL)
        return;
    time_now++;
    remaining_time_slice--;

    if(remaining_time_slice == 0) {
        if (current_process->burst == 0) {
            // terminate the child process
            printf("Scheduler: Time Now: %d seconds.\n", time_now);
            printf("Terminating Process %d ", current_process->id);
            fflush(stdout);
            kill(current_process->pid, SIGTERM);
        } else {
            // pause the child process
            printf("Scheduler: Time Now: %d seconds.\n", time_now);
            printf("Suspending Process %d and moving it to FCFS queue ", current_process->id);
            fflush(stdout);
            kill(current_process->pid, SIGTSTP);
        }

        schedule_process2();
    }
}

unsigned long long generateRandom10DigitNumber() {
    unsigned long long min = 1000000000;   // 10^9
    unsigned long long max = 9999999999;   // 10^10 - 1

    srand((unsigned)time(NULL));
    unsigned long long random10DigitNumber = (rand() % (max - min + 1)) + min;
    return random10DigitNumber;
}


void schedule_process1(Process* process, int display_time) {
    current_process->pid = fork();
    if (current_process->pid == 0) {
        // child
        char process_id_str[10];
        sprintf(process_id_str, "%d", process->id);

        char random_number_str[10];
        sprintf(random_number_str, "%llu", generateRandom10DigitNumber());

        execl("./primes", "./primes", process_id_str, random_number_str, (char*)NULL);
        perror("exec failed");
        exit(1);
    } else {
        // parent
        if(display_time) {
            printf("Scheduler: Time Now: %d seconds.\n", time_now);
            printf("Scheduling to Process %d (PID %d) for the time slice of %d seconds.\n", current_process->id, current_process->pid, remaining_time_slice);
        } else {
            printf("and scheduling Process %d (PID %d) for the time slice of %d seconds.\n", current_process->id, current_process->pid, remaining_time_slice);
        }
    }
}

void schedule_process2() {
    Process* next_process = NULL;

    if(current_process->burst > 0) {
        enqueue(queue2, current_process->id, current_process->burst, current_process->pid);
    }
    if (!isEmpty(queue1)) {
        next_process = dequeue(queue1);
        current_process = next_process;

        remaining_time_slice = (current_process->burst < time_slice) ? current_process->burst : time_slice;
        current_process->burst -= remaining_time_slice;
        schedule_process1(next_process, 0);
        return;
    } else if (!isEmpty(queue2)) {
        static int msg_printed = 0;
        if(!msg_printed) {
            printf("\nNO MORE PROCESSES IN QUEUE 1. MOVING TO QUEUE 2.\n");
            msg_printed = 1;
        }

        next_process = dequeue(queue2);
        current_process = next_process;

        printf("Resuming Process %d (PID %d).\n", next_process->id, next_process->pid);

        remaining_time_slice = current_process->burst;
        current_process->burst = 0;
    } else {
        wait(0);
        finished_scheduling = 1;
    }
}


int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <time_slice>\n", argv[0]);
        exit(1);
    }

    time_slice = atoi(argv[2]);
    queue1 = createQueue();
    queue2 = createQueue();

    FILE* input_file = fopen(argv[1], "r");
    if (input_file == NULL) {
        perror("Error opening input file");
        exit(1);
    }

    char line[100];
    int process_id;
    int burst_time;

    fgets(line, sizeof(line), input_file);  // skip the header line
    while (fgets(line, sizeof(line), input_file)) {
        sscanf(line, "%d%*[^0-9]%d", &process_id, &burst_time);
        enqueue(queue1, process_id, burst_time, -1);
    }

    fclose(input_file);

    Process* process = dequeue(queue1);
    current_process = process;
    remaining_time_slice = (current_process->burst < time_slice) ? current_process->burst : time_slice;
    current_process->burst -= remaining_time_slice;

    // set up the timer
    struct itimerval timer;
    struct sigaction sa;
    sa.sa_handler = handle_SIGALRM;
    sigaction(SIGALRM, &sa, NULL);
    timer.it_value.tv_sec = 1;
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_sec = 1;
    timer.it_interval.tv_usec = 0;
    setitimer(ITIMER_REAL, &timer, NULL);

    schedule_process1(process, 1);

    while (!finished_scheduling) {
        continue;
    }

    // clean up
    destroyQueue(queue1);
    destroyQueue(queue2);

    printf("\nScheduler: No more processes to run. Bye\n");
    return 0;
}
