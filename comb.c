COPY

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>

#define BUFFER_SIZE 4096

void copy_file(const char *src, const char *dest) {
    int src_fd, dest_fd;
    ssize_t bytes_read, bytes_written;
    char buffer[BUFFER_SIZE];

    src_fd = open(src, O_RDONLY);
    if (src_fd < 0) {
        perror("Error opening source file");
        exit(EXIT_FAILURE);
    }

    dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd < 0) {
        perror("Error opening destination file");
        close(src_fd);
        exit(EXIT_FAILURE);
    }

    while ((bytes_read = read(src_fd, buffer, BUFFER_SIZE)) > 0) {
        bytes_written = write(dest_fd, buffer, bytes_read);
        if (bytes_written != bytes_read) {
            perror("Error writing to destination file");
            close(src_fd);
            close(dest_fd);
            exit(EXIT_FAILURE);
        }
    }

    if (bytes_read < 0) {
        perror("Error reading source file");
    }

    close(src_fd);
    close(dest_fd);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
        return EXIT_FAILURE;
    }

    copy_file(argv[1], argv[2]);
    return EXIT_SUCCESS;
}

WC

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>

/* Function to count lines, words, and characters in a file */
void count_stats(FILE *file, const char *filename, long *lines, long *words, long *chars) {
    int c;
    bool in_word = false;
    
    *lines = 0;
    *words = 0;
    *chars = 0;
    
    while ((c = fgetc(file)) != EOF) {
        (*chars)++;
        
        if (c == '\n')
            (*lines)++;
            
        /* Check for word boundaries */
        if (isspace(c)) {
            in_word = false;
        } else if (!in_word) {
            in_word = true;
            (*words)++;
        }
    }
}

int main(int argc, char *argv[]) {
    FILE *file;
    long total_lines = 0, total_words = 0, total_chars = 0;
    long lines, words, chars;
    
    /* If no files are specified, read from stdin */
    if (argc == 1) {
        count_stats(stdin, "stdin", &lines, &words, &chars);
        printf(" %7ld %7ld %7ld\n", lines, words, chars);
        return 0;
    }
    
    /* Process each file specified on the command line */
    for (int i = 1; i < argc; i++) {
        file = fopen(argv[i], "r");
        
        if (file == NULL) {
            fprintf(stderr, "wc: %s: No such file or directory\n", argv[i]);
            continue;
        }
        
        count_stats(file, argv[i], &lines, &words, &chars);
        printf(" %7ld %7ld %7ld %s\n", lines, words, chars, argv[i]);
        
        total_lines += lines;
        total_words += words;
        total_chars += chars;
        
        fclose(file);
    }
    
    /* Print totals if more than one file was processed */
    if (argc > 2)
        printf(" %7ld %7ld %7ld total\n", total_lines, total_words, total_chars);
    
    return 0;
}


PS

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>

void list_processes() {
    struct dirent *entry;
    DIR *proc_dir = opendir("/proc");

    if (!proc_dir) {
        perror("Failed to open /proc");
        exit(EXIT_FAILURE);
    }

    printf("%-10s %-20s\n", "PID", "COMMAND");
    printf("----------------------------\n");

    while ((entry = readdir(proc_dir)) != NULL) {
        if (!isdigit(entry->d_name[0]))  // Skip non-numeric entries
            continue;

        char cmd_path[256], cmd[256];
        FILE *cmd_file;

        snprintf(cmd_path, sizeof(cmd_path), "/proc/%s/cmdline", entry->d_name);
        cmd_file = fopen(cmd_path, "r");

        if (cmd_file) {
            if (fgets(cmd, sizeof(cmd), cmd_file) != NULL) {
                printf("%-10s %-20s\n", entry->d_name, cmd);
            } else {
                printf("%-10s [Unknown]\n", entry->d_name);
            }
            fclose(cmd_file);
        } else {
            printf("%-10s [Unknown]\n", entry->d_name);
        }
    }

    closedir(proc_dir);
}

int main() {
    list_processes();
    return EXIT_SUCCESS;
}



PRODUCER CONSUMER

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/wait.h>

#define BUFFER_SIZE 5   // Buffer size
#define SHM_KEY 1234    // Shared memory key
#define NUM_CONSUMERS 3 // Number of consumers
#define TOTAL_ITEMS 10  // Total items to be produced

// Shared buffer structure
typedef struct {
    int buffer[BUFFER_SIZE];
    int in, out ,sum;
    int items_produced;  // Track total items produced
    int items_consumed;  // Track total items consumed
    int done;            // Flag to indicate production is complete
    sem_t empty;         // Semaphore for empty slots
    sem_t full;          // Semaphore for filled slots
    sem_t mutex;         // Semaphore for mutual exclusion
} SharedBuffer;

int main() {
    int shmid;
    SharedBuffer *shm_ptr;

    // Create shared memory
    shmid = shmget(SHM_KEY, sizeof(SharedBuffer), IPC_CREAT | 0666);
    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    // Attach shared memory
    shm_ptr = (SharedBuffer *)shmat(shmid, NULL, 0);
    if (shm_ptr == (void *)-1) {
        perror("shmat failed");
        exit(1);
    }

    // Initialize buffer variables
    shm_ptr->in = 0;
    shm_ptr->out = 0;
    shm_ptr->sum = 0;
    shm_ptr->items_produced = 0;
    shm_ptr->items_consumed = 0;
    shm_ptr->done = 0;  // Initialize done flag

    // Initialize semaphores with `pshared=1` (shared between processes)
    sem_init(&shm_ptr->empty, 1, BUFFER_SIZE); // BUFFER_SIZE empty slots
    sem_init(&shm_ptr->full, 1, 0);            // 0 full slots initially
    sem_init(&shm_ptr->mutex, 1, 1);           // Mutex for mutual exclusion

    pid_t producer_pid = fork();

    if (producer_pid == -1) {
        perror("fork failed");
        exit(1);
    }

    if (producer_pid == 0) { 
        // 👷 Producer Process
        for (int i = 1; i <= TOTAL_ITEMS; i++) {
            sem_wait(&shm_ptr->empty);
            sem_wait(&shm_ptr->mutex);

            // Produce item
            shm_ptr->buffer[shm_ptr->in] = i;
            shm_ptr->items_produced++;
            printf("Produced: %d\n", i);
            fflush(stdout);
            shm_ptr->in = (shm_ptr->in + 1) % BUFFER_SIZE;

            sem_post(&shm_ptr->mutex);
            sem_post(&shm_ptr->full);

            sleep(1);
        }
        
        // Signal that production is complete
        sem_wait(&shm_ptr->mutex);
        shm_ptr->done = 1;
        sem_post(&shm_ptr->mutex);
        
        // Wake up any waiting consumers
        for (int i = 0; i < NUM_CONSUMERS; i++) {
            sem_post(&shm_ptr->full);
        }
        
        exit(0);
    } 

    // Create multiple consumer processes
    pid_t consumer_pids[NUM_CONSUMERS];
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        consumer_pids[i] = fork();
        if (consumer_pids[i] == -1) {
            perror("fork failed");
            exit(1);
        }

        if (consumer_pids[i] == 0) { 
            // 👶 Consumer Process
            while (1) {
                sem_wait(&shm_ptr->full);
                sem_wait(&shm_ptr->mutex);

                // Check if all items have been consumed
                if (shm_ptr->done && shm_ptr->items_consumed >= TOTAL_ITEMS) {
                    sem_post(&shm_ptr->mutex);
                    // Do NOT post to full semaphore here
                    break; // Exit loop
                }

                // Consume item
                int item = shm_ptr->buffer[shm_ptr->out];
                shm_ptr->sum += item;
                shm_ptr->items_consumed++;
                printf("\tConsumer %d consumed: %d\n", i + 1, item);
                fflush(stdout);
                shm_ptr->out = (shm_ptr->out + 1) % BUFFER_SIZE;

                sem_post(&shm_ptr->mutex);
                sem_post(&shm_ptr->empty);

                sleep(2);
            }
            exit(0);
        }
    }

    // Wait for producer to finish
    waitpid(producer_pid, NULL, 0);

    // Wait for all consumers to finish
    for (int i = 0; i < NUM_CONSUMERS; i++) {
        waitpid(consumer_pids[i], NULL, 0);
    }

    printf("\n\nTotal sum = %d\n",shm_ptr->sum);

    // Cleanup: Destroy semaphores and shared memory
    sem_destroy(&shm_ptr->empty);
    sem_destroy(&shm_ptr->full);
    sem_destroy(&shm_ptr->mutex);
    shmdt(shm_ptr);
    shmctl(shmid, IPC_RMID, NULL);

    printf("Program completed successfully\n");
    return 0;
}


SCHEDULING


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
