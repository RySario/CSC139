/*
Ryan Sario
Section: 01
OS: macOS
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/shm.h>
#include <sys/timeb.h>
#include <semaphore.h>
#include <stdbool.h>

#define MAX_SIZE 100000000
#define MAX_PROCESSES 16
#define RANDOM_SEED 7649
#define MAX_RANDOM_NUMBER 3000
#define NUM_LIMIT 9973

// Global variables for shared memory
int *gData; // The array that will hold the data (shared memory)
int *gProcessProd; // Shared memory for the product of each process division
int *gDoneProcessCount; // Shared memory for counting done processes
int processCount; // Global variable for number of processes
long gRefTime; // Global variable for timing reference

// Semaphores in shared memory
sem_t *completed; // To notify parent that all processes have completed or one of them found a zero
sem_t *mutex; // Binary semaphore to protect access to shared data like gDoneProcessCount

// Function declarations
int SqFindProd(int size); // Sequential FindProduct (no processes)
void ProcessFindProd(int processNum, int startIdx, int endIdx); // Function for processes to compute the product
int ComputeTotalProduct(); // Multiply the division products to compute the total modular product
void InitSharedMemory(int size); // Initialize shared memory variables
void GenerateInput(int size, int indexForZero); // Generate the input array
void CalculateIndices(int arraySize, int processCnt, int indices[MAX_PROCESSES][3]); // Calculate the indices to divide the array into equal divisions
int GetRand(int min, int max); // Get a random number between min and max

// Timing functions
long GetMilliSecondTime(struct timeb timeBuf);
long GetCurrentTime(void);
void SetTime(void);
long GetTime(void);

// (Rest of the code follows...)


int main(int argc, char *argv[]) {
    int indices[MAX_PROCESSES][3];
    int i, indexForZero, arraySize, prod;
    pid_t pid[MAX_PROCESSES];

    // Code for parsing and checking command-line arguments
    if (argc != 4) {
        fprintf(stderr, "Invalid number of arguments!\n");
        exit(-1);
    }
    if ((arraySize = atoi(argv[1])) <= 0 || arraySize > MAX_SIZE) {
        fprintf(stderr, "Invalid Array Size\n");
        exit(-1);
    }
    processCount = atoi(argv[2]);
    if (processCount > MAX_PROCESSES || processCount <= 0) {
        fprintf(stderr, "Invalid Process Count\n");
        exit(-1);
    }
    indexForZero = atoi(argv[3]);
    if (indexForZero < -1 || indexForZero >= arraySize) {
        fprintf(stderr, "Invalid index for zero!\n");
        exit(-1);
    }

    // Initialize shared memory and generate input data
    InitSharedMemory(arraySize);
    GenerateInput(arraySize, indexForZero);
    CalculateIndices(arraySize, processCount, indices);

    // Sequential multiplication
    SetTime();
    prod = SqFindProd(arraySize);
    printf("Sequential multiplication completed in %ld ms. Product = %d\n", GetTime(), prod);

    // Process-based multiplication with parent waiting for all child processes
    SetTime();
    for (i = 0; i < processCount; i++) {
        pid[i] = fork();
        if (pid[i] == 0) { // Child process
            ProcessFindProd(i, indices[i][1], indices[i][2]);
            exit(0);
        }
    }

    // Parent process waits for all child processes to complete
    for (i = 0; i < processCount; i++) {
        wait(NULL);
    }
    prod = ComputeTotalProduct();
    printf("Process-based multiplication with parent waiting for all children completed in %ld ms. Product = %d\n", GetTime(), prod);

    // Cleanup shared memory and semaphores
    shmdt(gData);
    shmdt(gProcessProd);
    shmdt(gDoneProcessCount);
    shmdt(completed);
    shmdt(mutex);
    return 0;
}

// Sequential FindProduct (no processes)
int SqFindProd(int size) {
    int prod = 1;
    for (int i = 0; i < size; i++) {
        prod *= gData[i];
        prod %= NUM_LIMIT;
        if (gData[i] == 0) break; // Stop if zero is encountered
    }
    return prod;
}

// Process FindProduct
void ProcessFindProd(int processNum, int startIdx, int endIdx) {
    int prod = 1;

    // Compute the product for the assigned division
    for (int i = startIdx; i <= endIdx; i++) {
        prod *= gData[i];
        prod %= NUM_LIMIT;
        if (gData[i] == 0) { // If a zero is found, notify parent and exit
            gProcessProd[processNum] = 0;
            sem_post(completed); // Notify parent immediately
            exit(0);
        }
    }

    // Store the product for this process
    gProcessProd[processNum] = prod;

    // Protect access to gDoneProcessCount with mutex semaphore
    sem_wait(mutex);
    (*gDoneProcessCount)++;
    if (*gDoneProcessCount == processCount) {
        // All processes are done, so signal the parent
        sem_post(completed);
    }
    sem_post(mutex);
}


// Multiply the division products to compute the total modular product
int ComputeTotalProduct() {
    int i, prod = 1;
    for (i = 0; i < processCount; i++) {
        if (gProcessProd[i] == 0) {
            return 0; // If any process found a zero, the total product is zero
        }
        prod *= gProcessProd[i];
        prod %= NUM_LIMIT;
    }
    return prod;
}

// Initialize shared memory
// Initialize shared memory
void InitSharedMemory(int size) {
    int shmDataId = shmget(IPC_PRIVATE, sizeof(int) * size, IPC_CREAT | 0666);
    if (shmDataId < 0) {
        perror("shmget failed for gData");
        exit(1);
    }
    int shmProdId = shmget(IPC_PRIVATE, sizeof(int) * MAX_PROCESSES, IPC_CREAT | 0666);
    if (shmProdId < 0) {
        perror("shmget failed for gProcessProd");
        exit(1);
    }
    int shmDoneCountId = shmget(IPC_PRIVATE, sizeof(int), IPC_CREAT | 0666);
    if (shmDoneCountId < 0) {
        perror("shmget failed for gDoneProcessCount");
        exit(1);
    }
    int shmCompletedId = shmget(IPC_PRIVATE, sizeof(sem_t), IPC_CREAT | 0666);
    if (shmCompletedId < 0) {
        perror("shmget failed for completed semaphore");
        exit(1);
    }
    int shmMutexId = shmget(IPC_PRIVATE, sizeof(sem_t), IPC_CREAT | 0666);
    if (shmMutexId < 0) {
        perror("shmget failed for mutex semaphore");
        exit(1);
    }

    gData = (int*) shmat(shmDataId, NULL, 0);
    if (gData == (int*)-1) {
        perror("shmat failed for gData");
        exit(1);
    }
    gProcessProd = (int*) shmat(shmProdId, NULL, 0);
    if (gProcessProd == (int*)-1) {
        perror("shmat failed for gProcessProd");
        exit(1);
    }
    gDoneProcessCount = (int*) shmat(shmDoneCountId, NULL, 0);
    if (gDoneProcessCount == (int*)-1) {
        perror("shmat failed for gDoneProcessCount");
        exit(1);
    }
    completed = (sem_t*) shmat(shmCompletedId, NULL, 0);
    if (completed == (sem_t*)-1) {
        perror("shmat failed for completed semaphore");
        exit(1);
    }
    mutex = (sem_t*) shmat(shmMutexId, NULL, 0);
    if (mutex == (sem_t*)-1) {
        perror("shmat failed for mutex semaphore");
        exit(1);
    }

    sem_init(completed, 1, 0);
    sem_init(mutex, 1, 1);
    *gDoneProcessCount = 0;
}


// Generate the input array with random numbers, place zero if required
void GenerateInput(int size, int indexForZero) {
    srand(RANDOM_SEED);
    for (int i = 0; i < size; i++) {
        gData[i] = GetRand(1, MAX_RANDOM_NUMBER);
    }
    if (indexForZero >= 0 && indexForZero < size) {
        gData[indexForZero] = 0;
    }
}

// Calculate the right indices to divide the array into process count equal divisions
void CalculateIndices(int arraySize, int processCnt, int indices[MAX_PROCESSES][3]) {
    int chunkSize = arraySize / processCnt;
    for (int i = 0; i < processCnt; i++) {
        indices[i][0] = i;
        indices[i][1] = i * chunkSize;
        indices[i][2] = (i == processCnt - 1) ? arraySize - 1 : (i + 1) * chunkSize - 1;
    }
}

// Get a random number in the range [x, y]
int GetRand(int x, int y) {
    int r = rand();
    r = x + r % (y - x + 1);
    return r;
}

// Timing functions
long GetMilliSecondTime(struct timeb timeBuf) {
    long mliScndTime;
    mliScndTime = timeBuf.time;
    mliScndTime *= 1000;
    mliScndTime += timeBuf.millitm;
    return mliScndTime;
}

long GetCurrentTime(void) {
    long crntTime = 0;
    struct timeb timeBuf;
    ftime(&timeBuf);
    crntTime = GetMilliSecondTime(timeBuf);
    return crntTime;
}

void SetTime(void) {
    gRefTime = GetCurrentTime();
}

long GetTime(void) {
    long crntTime = GetCurrentTime();
    return (crntTime - gRefTime);
}
