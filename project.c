#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 100
#define BLOCK_COUNT 5 // Number of fixed memory blocks

typedef enum {
    READY,
    RUNNING,
    TERMINATED
} ProcessState;

typedef struct {
    int pid;
    int arrival;
    int burst;
    int priority;
    int start;
    int completion;
    int turnaround;
    int waiting;
    int response;
    int memory_block;  // Assigned memory block
    ProcessState state;
} Process;

Process proc[MAX], copy[MAX];
int n = 0;

// Memory blocks for First-Fit allocation
int memory_blocks[BLOCK_COUNT] = {100, 500, 200, 300, 600};
int allocated_blocks[BLOCK_COUNT] = {0}; // 0 = Free, 1 = Allocated

// First-Fit Allocation Function
int first_fit(int burst) {
    for (int i = 0; i < BLOCK_COUNT; i++) {
        if (!allocated_blocks[i] && memory_blocks[i] >= burst) {
            allocated_blocks[i] = 1;
            return i; // Return index of allocated block
        }
    }
    return -1; // No suitable block found
}

// Free Memory Block After Process Completes
void free_memory_block(int block_index) {
    if (block_index != -1) {
        allocated_blocks[block_index] = 0; // Mark block as free
    }
}

void read_file() {
    FILE *file = fopen("processes.txt", "r");
    if (!file) {
        printf("Error: Cannot open file.\n");
        exit(1);
    }
    fscanf(file, "%*s %*s %*s %*s"); // Skip header
    n = 0;
    while (fscanf(file, "%d %d %d %d", &proc[n].pid, &proc[n].arrival, &proc[n].burst, &proc[n].priority) != EOF) {
        proc[n].state = READY;
        proc[n].response = -1;
        proc[n].memory_block = -1; // No memory assigned yet
        copy[n] = proc[n];
        n++;
    }
    fclose(file);
}

void print_table(Process p[]) {
    printf("\n%-5s %-12s %-10s %-8s\n", 
           "PID", "Arrival", "Burst", "Priority");
    printf("-------------------------------------\n");

    for (int i = 0; i < n; i++) {
        const char *state_str = (p[i].state == READY) ? "READY" :
                                (p[i].state == RUNNING) ? "RUNNING" : "TERMINATED";
        printf("%-5d %-12d %-10d %-8d\n", 
               p[i].pid, p[i].arrival, p[i].burst, p[i].priority);
    }
}

void print_results(Process p[], float avg_wt, float avg_tat, float avg_rt) {
    printf("\n%-5s %-8s %-7s %-8s %-10s %-10s %-10s\n",
           "PID", "Arrival", "Burst", "Response", "Waiting", "Turnaround", "State");
    for (int i = 0; i < n; i++) {
        const char *state_str = (p[i].state == READY) ? "READY" :
                                (p[i].state == RUNNING) ? "RUNNING" : "TERMINATED";
        printf("P%-4d %-8d %-7d %-8d %-10d %-10d %-12s\n",
               p[i].pid, p[i].arrival, p[i].burst, p[i].response,
               p[i].waiting, p[i].turnaround, state_str);
    }
    printf("\nAverage Response Time: %.2f\n", avg_rt);
    printf("Average Waiting Time: %.2f\n", avg_wt);
    printf("Average Turnaround Time: %.2f\n", avg_tat);
}

// FCFS Scheduling with First-Fit Allocation
void fcfs() {
    float total_wt = 0, total_tat = 0, total_rt = 0;
    int time = 0;
    memcpy(proc, copy, sizeof(copy));

    printf("\n--- FCFS Scheduling with First-Fit Allocation ---\n");
    printf("Gantt Chart:\n|");

    for (int i = 0; i < n; i++) {
        if (time < proc[i].arrival) time = proc[i].arrival;

        // Allocate memory before executing the process
        proc[i].memory_block = first_fit(proc[i].burst);
        if (proc[i].memory_block == -1) {
            printf("\nP%d could not be allocated memory!\n", proc[i].pid);
            continue; // Skip execution if no memory available
        }

        proc[i].start = time;
        proc[i].response = proc[i].start - proc[i].arrival;
        proc[i].state = RUNNING;
        printf(" P%d |", proc[i].pid);

        time += proc[i].burst;
        proc[i].completion = time;
        proc[i].turnaround = proc[i].completion - proc[i].arrival;
        proc[i].waiting = proc[i].turnaround - proc[i].burst;
        proc[i].state = TERMINATED;

        // Free memory after execution
        free_memory_block(proc[i].memory_block);

        total_rt += proc[i].response;
        total_wt += proc[i].waiting;
        total_tat += proc[i].turnaround;
    }

    printf("\n0");
    for (int i = 0; i < n; i++) printf("   %d", proc[i].completion);
    print_results(proc, total_wt / n, total_tat / n, total_rt / n);
}

// SJF Scheduling with First-Fit Allocation
void sjf() {
    float total_wt = 0, total_tat = 0, total_rt = 0;
    int completed = 0, time = 0;
    int is_completed[MAX] = {0};
    memcpy(proc, copy, sizeof(copy));

    printf("\n--- SJF Scheduling with First-Fit Allocation ---\n");
    printf("Gantt Chart:\n|");

    while (completed < n) {
        int idx = -1, min_burst = 9999;
        for (int i = 0; i < n; i++) {
            if (proc[i].arrival <= time && !is_completed[i] && proc[i].burst < min_burst) {
                min_burst = proc[i].burst;
                idx = i;
            }
        }

        if (idx != -1) {
            // Allocate memory
            proc[idx].memory_block = first_fit(proc[idx].burst);
            if (proc[idx].memory_block == -1) {
                printf("\nP%d could not be allocated memory!\n", proc[idx].pid);
                continue;
            }

            proc[idx].start = time;
            proc[idx].response = proc[idx].start - proc[idx].arrival;
            proc[idx].state = RUNNING;
            printf(" P%d |", proc[idx].pid);

            time += proc[idx].burst;
            proc[idx].completion = time;
            proc[idx].turnaround = proc[idx].completion - proc[idx].arrival;
            proc[idx].waiting = proc[idx].turnaround - proc[idx].burst;
            proc[idx].state = TERMINATED;
            is_completed[idx] = 1;
            completed++;

            // Free memory
            free_memory_block(proc[idx].memory_block);

            total_rt += proc[idx].response;
            total_wt += proc[idx].waiting;
            total_tat += proc[idx].turnaround;
        } else {
            time++;
        }
    }

    printf("\n0");
    for (int i = 0; i < n; i++) printf("   %d", proc[i].completion);
    print_results(proc, total_wt / n, total_tat / n, total_rt / n);
}

// Priority Scheduling with First-Fit Allocation
void priority() {
    float total_wt = 0, total_tat = 0, total_rt = 0;
    int completed = 0, time = 0;
    int is_completed[MAX] = {0};
    memcpy(proc, copy, sizeof(copy));

    printf("\n--- Priority Scheduling with First-Fit Allocation ---\n");
    printf("Gantt Chart:\n|");

    while (completed < n) {
        int idx = -1, highest_priority = 999999;
        for (int i = 0; i < n; i++) {
            if (proc[i].arrival <= time && !is_completed[i] && proc[i].priority < highest_priority) {
                highest_priority = proc[i].priority;
                idx = i;
            }
        }

        if (idx != -1) {
            // Allocate memory
            proc[idx].memory_block = first_fit(proc[idx].burst);
            if (proc[idx].memory_block == -1) {
                printf("\nP%d could not be allocated memory!\n", proc[idx].pid);
                continue;
            }

            // Print Gantt Chart Entry
            printf(" P%d |", proc[idx].pid);

            proc[idx].start = time;
            proc[idx].response = proc[idx].start - proc[idx].arrival;
            proc[idx].state = RUNNING;

            time += proc[idx].burst;
            proc[idx].completion = time;
            proc[idx].turnaround = proc[idx].completion - proc[idx].arrival;
            proc[idx].waiting = proc[idx].turnaround - proc[idx].burst;
            proc[idx].state = TERMINATED;
            is_completed[idx] = 1;
            completed++;

            // Free memory after execution
            free_memory_block(proc[idx].memory_block);

            total_rt += proc[idx].response;
            total_wt += proc[idx].waiting;
            total_tat += proc[idx].turnaround;
        } else {
            time++;
        }
    }

    printf("\n0"); 
    for (int i = 0; i < n; i++) printf("   %d", proc[i].completion);
    print_results(proc, total_wt / n, total_tat / n, total_rt / n);
}


int main() {
    int choice;
    read_file();

    printf("Process Table:");
    print_table(proc);
    
    while (1) {
        // Reset memory allocation tracking
        for (int i = 0; i < BLOCK_COUNT; i++) {
            allocated_blocks[i] = 0;  // Ensure all memory blocks are available before each scheduling run
        }

        printf("\n--- CPU Scheduling Menu (With First-Fit Memory Allocation) ---\n");
        printf("1. First-Come, First-Served (FCFS)\n");
        printf("2. Shortest Job First (SJF)\n");
        printf("3. Priority Scheduling\n");
        printf("4. Exit\n");
        printf("Select an option: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                fcfs();
                break;
            case 2:
                sjf();
                break;
            case 3:
                priority();
                break;
            case 4:
                printf("Exiting...\n");
                return 0;
            default:
                printf("Invalid option. Try again.\n");
        }

    }

    return 0;
}
