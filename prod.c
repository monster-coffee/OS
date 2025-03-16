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