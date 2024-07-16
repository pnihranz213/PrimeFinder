#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

unsigned long long int current_number = 0;
unsigned long long int highest_prime = 0;
int process_id = 0;

void handle_SIGTSTP(int) {
    printf("Process %d: my PID is %d: I am about to be suspended... Highest prime number I found is %llu.\n", process_id, getpid(), highest_prime);
}

void handle_SIGCONT(int) {}

void handle_SIGTERM(int) {
    printf("Process %d: my PID is %d: I am leaving the system. The largest prime I found was %llu.\n", process_id, getpid(), highest_prime);
    exit(0);
}

int is_prime(unsigned long long int n) {
    for(unsigned long long int i=2; i<n; ++i) {
        if(n%i == 0)
            return 0;
    }

    return 1;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <process_id> <starting_number>\n", argv[0]);
        exit(1);
    }

    signal(SIGTSTP, handle_SIGTSTP);
    signal(SIGCONT, handle_SIGCONT);
    signal(SIGTERM, handle_SIGTERM);

    process_id = atoi(argv[1]);
    current_number = atoll(argv[2]);

    printf("Process %d: my PID is %d: I just got started. I am starting with the number %llu to find the next prime number.\n", process_id, getpid(), current_number);

    highest_prime = current_number;
    while (1) {
        if (is_prime(current_number++)) {
            highest_prime = current_number;
        }
    }

    return 0;
}
