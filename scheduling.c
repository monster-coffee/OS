#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

struct Process {
    int pid;          // Process ID
    int arrival_time; // Arrival time
    int burst_time;   // Burst time
    int priority;     // Priority (lower value means higher priority)
    int remaining_time; // Remaining burst time
    int waiting_time;   // Waiting time
    int turnaround_time; // Turnaround time
    int completion_time; // Completion time
};

// Function to calculate waiting time and turnaround time for each process
void calculate_times(struct Process *processes, int n) {
    for (int i = 0; i < n; i++) {
        processes[i].turnaround_time = processes[i].completion_time - processes[i].arrival_time;
        processes[i].waiting_time = processes[i].turnaround_time - processes[i].burst_time;
    }
}

// Function to display results
void display_results(struct Process *processes, int n) {
    float avg_waiting_time = 0, avg_turnaround_time = 0;
    
    printf("\nProcess\tArrival Time\tBurst Time\tPriority\tWaiting Time\tTurnaround Time\n");
    for (int i = 0; i < n; i++) {
        printf("%d\t%d\t\t%d\t\t%d\t\t%d\t\t%d\n", 
               processes[i].pid, 
               processes[i].arrival_time, 
               processes[i].burst_time, 
               processes[i].priority, 
               processes[i].waiting_time, 
               processes[i].turnaround_time);
        
        avg_waiting_time += processes[i].waiting_time;
        avg_turnaround_time += processes[i].turnaround_time;
    }
    
    avg_waiting_time /= n;
    avg_turnaround_time /= n;
    
    printf("\nAverage Waiting Time: %.2f", avg_waiting_time);
    printf("\nAverage Turnaround Time: %.2f\n", avg_turnaround_time);
}

// First Come First Serve (FCFS) scheduling
void fcfs(struct Process *processes, int n) {
    // Sort processes based on arrival time
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (processes[j].arrival_time > processes[j + 1].arrival_time) {
                struct Process temp = processes[j];
                processes[j] = processes[j + 1];
                processes[j + 1] = temp;
            }
        }
    }
    
    // Calculate completion time for each process
    int current_time = 0;
    for (int i = 0; i < n; i++) {
        // Update current time
        if (current_time < processes[i].arrival_time) {
            current_time = processes[i].arrival_time;
        }
        
        // Process execution
        current_time += processes[i].burst_time;
        processes[i].completion_time = current_time;
    }
    
    // Calculate waiting time and turnaround time
    calculate_times(processes, n);
    
    printf("\n--- First Come First Serve (FCFS) Scheduling ---\n");
    display_results(processes, n);
}

// Non-Preemptive Shortest Job First (SJF) scheduling
void sjf_non_preemptive(struct Process *processes, int n) {
    struct Process temp[n];
    for (int i = 0; i < n; i++) {
        temp[i] = processes[i];
    }
    
    int current_time = 0;
    int completed = 0;
    
    while (completed < n) {
        int shortest_job = -1;
        int shortest_time = INT_MAX;
        
        // Find the process with the shortest burst time that has arrived
        for (int i = 0; i < n; i++) {
            if (temp[i].arrival_time <= current_time && temp[i].burst_time > 0) {
                if (temp[i].burst_time < shortest_time) {
                    shortest_time = temp[i].burst_time;
                    shortest_job = i;
                }
            }
        }
        
        // If no process is available, advance the time
        if (shortest_job == -1) {
            current_time++;
            continue;
        }
        
        // Execute the shortest job
        current_time += temp[shortest_job].burst_time;
        temp[shortest_job].burst_time = 0;
        temp[shortest_job].completion_time = current_time;
        completed++;
    }
    
    // Calculate waiting time and turnaround time
    calculate_times(temp, n);
    
    printf("\n--- Non-Preemptive Shortest Job First (SJF) Scheduling ---\n");
    display_results(temp, n);
}

// Preemptive Shortest Job First (SJF) scheduling
void sjf_preemptive(struct Process *processes, int n) {
    struct Process temp[n];
    for (int i = 0; i < n; i++) {
        temp[i] = processes[i];
        temp[i].remaining_time = temp[i].burst_time;
    }
    
    int current_time = 0;
    int completed = 0;
    
    while (completed < n) {
        int shortest_job = -1;
        int shortest_time = INT_MAX;
        
        // Find the process with the shortest remaining time that has arrived
        for (int i = 0; i < n; i++) {
            if (temp[i].arrival_time <= current_time && temp[i].remaining_time > 0) {
                if (temp[i].remaining_time < shortest_time) {
                    shortest_time = temp[i].remaining_time;
                    shortest_job = i;
                }
            }
        }
        
        // If no process is available, advance the time
        if (shortest_job == -1) {
            current_time++;
            continue;
        }
        
        // Execute the process for 1 time unit
        temp[shortest_job].remaining_time--;
        current_time++;
        
        // If the process is completed
        if (temp[shortest_job].remaining_time == 0) {
            completed++;
            temp[shortest_job].completion_time = current_time;
        }
    }
    
    // Calculate waiting time and turnaround time
    calculate_times(temp, n);
    
    printf("\n--- Preemptive Shortest Job First (SJF) Scheduling ---\n");
    display_results(temp, n);
}

// Non-Preemptive Longest Job First (LJF) scheduling
void ljf_non_preemptive(struct Process *processes, int n) {
    struct Process temp[n];
    for (int i = 0; i < n; i++) {
        temp[i] = processes[i];
    }
    
    int current_time = 0;
    int completed = 0;
    
    while (completed < n) {
        int longest_job = -1;
        int longest_time = -1;
        
        // Find the process with the longest burst time that has arrived
        for (int i = 0; i < n; i++) {
            if (temp[i].arrival_time <= current_time && temp[i].burst_time > 0) {
                if (temp[i].burst_time > longest_time) {
                    longest_time = temp[i].burst_time;
                    longest_job = i;
                }
            }
        }
        
        // If no process is available, advance the time
        if (longest_job == -1) {
            current_time++;
            continue;
        }
        
        // Execute the longest job
        current_time += temp[longest_job].burst_time;
        temp[longest_job].burst_time = 0;
        temp[longest_job].completion_time = current_time;
        completed++;
    }
    
    // Calculate waiting time and turnaround time
    calculate_times(temp, n);
    
    printf("\n--- Non-Preemptive Longest Job First (LJF) Scheduling ---\n");
    display_results(temp, n);
}

// Non-Preemptive Priority scheduling
void priority_non_preemptive(struct Process *processes, int n) {
    struct Process temp[n];
    for (int i = 0; i < n; i++) {
        temp[i] = processes[i];
    }
    
    int current_time = 0;
    int completed = 0;
    
    while (completed < n) {
        int highest_priority_job = -1;
        int highest_priority = INT_MAX;
        
        // Find the process with the highest priority (lowest value) that has arrived
        for (int i = 0; i < n; i++) {
            if (temp[i].arrival_time <= current_time && temp[i].burst_time > 0) {
                if (temp[i].priority < highest_priority) {
                    highest_priority = temp[i].priority;
                    highest_priority_job = i;
                }
            }
        }
        
        // If no process is available, advance the time
        if (highest_priority_job == -1) {
            current_time++;
            continue;
        }
        
        // Execute the highest priority job
        current_time += temp[highest_priority_job].burst_time;
        temp[highest_priority_job].burst_time = 0;
        temp[highest_priority_job].completion_time = current_time;
        completed++;
    }
    
    // Calculate waiting time and turnaround time
    calculate_times(temp, n);
    
    printf("\n--- Non-Preemptive Priority Scheduling ---\n");
    display_results(temp, n);
}

// Preemptive Priority scheduling
void priority_preemptive(struct Process *processes, int n) {
    struct Process temp[n];
    for (int i = 0; i < n; i++) {
        temp[i] = processes[i];
        temp[i].remaining_time = temp[i].burst_time;
    }
    
    int current_time = 0;
    int completed = 0;
    
    while (completed < n) {
        int highest_priority_job = -1;
        int highest_priority = INT_MAX;
        
        // Find the process with the highest priority (lowest value) that has arrived
        for (int i = 0; i < n; i++) {
            if (temp[i].arrival_time <= current_time && temp[i].remaining_time > 0) {
                if (temp[i].priority < highest_priority) {
                    highest_priority = temp[i].priority;
                    highest_priority_job = i;
                }
            }
        }
        
        // If no process is available, advance the time
        if (highest_priority_job == -1) {
            current_time++;
            continue;
        }
        
        // Execute the process for 1 time unit
        temp[highest_priority_job].remaining_time--;
        current_time++;
        
        // If the process is completed
        if (temp[highest_priority_job].remaining_time == 0) {
            completed++;
            temp[highest_priority_job].completion_time = current_time;
        }
    }
    
    // Calculate waiting time and turnaround time
    calculate_times(temp, n);
    
    printf("\n--- Preemptive Priority Scheduling ---\n");
    display_results(temp, n);
}

// Round Robin scheduling
void round_robin(struct Process *processes, int n, int quantum) {
    struct Process temp[n];
    for (int i = 0; i < n; i++) {
        temp[i] = processes[i];
        temp[i].remaining_time = temp[i].burst_time;
    }
    
    // Sort processes based on arrival time
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - i - 1; j++) {
            if (temp[j].arrival_time > temp[j + 1].arrival_time) {
                struct Process swap = temp[j];
                temp[j] = temp[j + 1];
                temp[j + 1] = swap;
            }
        }
    }
    
    int current_time = 0;
    int completed = 0;
    int queue[100];
    int front = 0, rear = 0;
    
    // Add the first process to the queue
    queue[rear++] = 0;
    
    while (completed < n) {
        int current_process = -1;
        
        // If there are processes in the queue
        if (front < rear) {
            current_process = queue[front++];
            
            // Execute the process for quantum time or until completion
            int execution_time = (temp[current_process].remaining_time < quantum) ? 
                                  temp[current_process].remaining_time : quantum;
            
            temp[current_process].remaining_time -= execution_time;
            current_time += execution_time;
            
            // Check for new arrivals during execution
            for (int i = 0; i < n; i++) {
                if (i != current_process && 
                    temp[i].arrival_time <= current_time && 
                    temp[i].remaining_time > 0 && 
                    temp[i].arrival_time > current_time - execution_time) {
                    queue[rear++] = i;
                }
            }
            
            // If the process is not completed, add it back to the queue
            if (temp[current_process].remaining_time > 0) {
                queue[rear++] = current_process;
            } else {
                // Process completed
                temp[current_process].completion_time = current_time;
                completed++;
            }
        } else {
            // No process in the queue, find the next arriving process
            int next_arrival_time = INT_MAX;
            int next_process = -1;
            
            for (int i = 0; i < n; i++) {
                if (temp[i].remaining_time > 0 && temp[i].arrival_time < next_arrival_time) {
                    next_arrival_time = temp[i].arrival_time;
                    next_process = i;
                }
            }
            
            if (next_process != -1) {
                current_time = next_arrival_time;
                queue[rear++] = next_process;
            }
        }
    }
    
    // Calculate waiting time and turnaround time
    calculate_times(temp, n);
    
    printf("\n--- Round Robin Scheduling (Quantum = %d) ---\n", quantum);
    display_results(temp, n);
}

int main() {
    int n;
    int choice;
    int quantum;
    
    printf("Enter the number of processes: ");
    scanf("%d", &n);
    
    struct Process processes[n];
    
    // Input process details
    for (int i = 0; i < n; i++) {
        processes[i].pid = i + 1;
        
        printf("\nProcess %d\n", i + 1);
        printf("Enter arrival time: ");
        scanf("%d", &processes[i].arrival_time);
        
        printf("Enter burst time: ");
        scanf("%d", &processes[i].burst_time);
        
        printf("Enter priority (lower value means higher priority): ");
        scanf("%d", &processes[i].priority);
    }
    
    do {
        printf("\n\nCPU Scheduling Algorithms\n");
        printf("1. First Come First Serve (FCFS)\n");
        printf("2. Shortest Job First (SJF) - Non-Preemptive\n");
        printf("3. Shortest Job First (SJF) - Preemptive\n");
        printf("4. Longest Job First (LJF) - Non-Preemptive\n");
        printf("5. Priority Scheduling - Non-Preemptive\n");
        printf("6. Priority Scheduling - Preemptive\n");
        printf("7. Round Robin\n");
        printf("0. Exit\n");
        printf("Enter your choice: ");
        scanf("%d", &choice);
        
        switch (choice) {
            case 1:
                fcfs(processes, n);
                break;
            case 2:
                sjf_non_preemptive(processes, n);
                break;
            case 3:
                sjf_preemptive(processes, n);
                break;
            case 4:
                ljf_non_preemptive(processes, n);
                break;
            case 5:
                priority_non_preemptive(processes, n);
                break;
            case 6:
                priority_preemptive(processes, n);
                break;
            case 7:
                printf("Enter time quantum: ");
                scanf("%d", &quantum);
                round_robin(processes, n, quantum);
                break;
            case 0:
                printf("Exiting...\n");
                break;
            default:
                printf("Invalid choice! Please try again.\n");
        }
    } while (choice != 0);
    
    return 0;
}