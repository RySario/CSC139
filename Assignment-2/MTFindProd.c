/*
Ryan Sario
Section: 01
OS: macOS
*/
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/timeb.h>
#include <semaphore.h>
#include <stdbool.h> // This enables the use of bool in C

#define MAX_SIZE 100000000
#define MAX_THREADS 16
#define RANDOM_SEED 7649
#define MAX_RANDOM_NUMBER 3000
#define NUM_LIMIT 9973

// Global variables
long gRefTime; // For timing
int gData[MAX_SIZE]; // The array that will hold the data
int gThreadCount; // Number of threads
int gDoneThreadCount; // Number of threads that are done at a certain point
int gThreadProd[MAX_THREADS]; // The modular product for each array division that a single thread is responsible for
bool gThreadDone[MAX_THREADS]; // Is this thread done? Used when the parent is continually checking on child threads

// Semaphores
sem_t completed; // To notify parent that all threads have completed or one of them found a zero
sem_t mutex; // Binary semaphore to protect the shared variable gDoneThreadCount

// Function declarations
int SqFindProd(int size); // Sequential FindProduct (no threads)
void *ThFindProd(void *param); // Thread FindProduct without semaphores
void *ThFindProdWithSemaphore(void *param); // Thread FindProduct with semaphores
int ComputeTotalProduct(); // Multiply the division products to compute the total modular product
void InitSharedVars(); // Initialize shared variables
void GenerateInput(int size, int indexForZero); // Generate the input array
void CalculateIndices(int arraySize, int thrdCnt, int indices[MAX_THREADS][3]); // Calculate the indices to divide the array into T divisions
int GetRand(int min, int max); // Get a random number between min and max

// Timing functions
long GetMilliSecondTime(struct timeb timeBuf);
long GetCurrentTime(void);
void SetTime(void);
long GetTime(void);

int main(int argc, char *argv[]) {
    pthread_t tid[MAX_THREADS];
    pthread_attr_t attr[MAX_THREADS];
    int indices[MAX_THREADS][3];
    int i, indexForZero, arraySize, prod;

    // Code for parsing and checking command-line arguments
    if (argc != 4) {
        fprintf(stderr, "Invalid number of arguments!\n");
        exit(-1);
    }
    if ((arraySize = atoi(argv[1])) <= 0 || arraySize > MAX_SIZE) {
        fprintf(stderr, "Invalid Array Size\n");
        exit(-1);
    }
    gThreadCount = atoi(argv[2]);
    if (gThreadCount > MAX_THREADS || gThreadCount <= 0) {
        fprintf(stderr, "Invalid Thread Count\n");
        exit(-1);
    }
    indexForZero = atoi(argv[3]);
    if (indexForZero < -1 || indexForZero >= arraySize) {
        fprintf(stderr, "Invalid index for zero!\n");
        exit(-1);
    }

    GenerateInput(arraySize, indexForZero);
    CalculateIndices(arraySize, gThreadCount, indices);

    // Sequential multiplication
    SetTime();
    prod = SqFindProd(arraySize);
    printf("Sequential multiplication completed in %ld ms. Product = %d\n", GetTime(), prod);

    // Threaded with parent waiting for all child threads
    InitSharedVars();
    SetTime();
    
    // Initialize threads and create threads, wait for all using pthread_join
    for (i = 0; i < gThreadCount; i++) {
        pthread_attr_init(&attr[i]);
        pthread_create(&tid[i], &attr[i], ThFindProd, (void*)indices[i]);
    }
    for (i = 0; i < gThreadCount; i++) {
        pthread_join(tid[i], NULL);
    }
    prod = ComputeTotalProduct();
    printf("Threaded multiplication with parent waiting for all children completed in %ld ms. Product = %d\n", GetTime(), prod);

    // Multi-threaded with busy waiting
    InitSharedVars();
    SetTime();
    
    // Initialize threads without semaphores, busy waiting
    for (i = 0; i < gThreadCount; i++) {
        pthread_attr_init(&attr[i]);
        pthread_create(&tid[i], &attr[i], ThFindProd, (void*)indices[i]);
    }
    // Busy waiting loop
    while (1) {
        int allDone = 1;
        for (i = 0; i < gThreadCount; i++) {
            if (!gThreadDone[i]) { // Check if any thread is still working
                allDone = 0;
                break;
            }
        }
        if (allDone) break; // Break out of loop if all threads are done
    }
    prod = ComputeTotalProduct();
    printf("Threaded multiplication with parent continually checking on children completed in %ld ms. Product = %d\n", GetTime(), prod);

    // Multi-threaded with semaphores
    InitSharedVars();
    sem_init(&completed, 0, 0);
    sem_init(&mutex, 0, 1);
    SetTime();
    
    // Initialize threads with semaphores
    for (i = 0; i < gThreadCount; i++) {
        pthread_attr_init(&attr[i]);
        pthread_create(&tid[i], &attr[i], ThFindProdWithSemaphore, (void*)indices[i]);
    }
    sem_wait(&completed);
    prod = ComputeTotalProduct();
    printf("Threaded multiplication with parent waiting on a semaphore completed in %ld ms. Prod = %d\n", GetTime(), prod);

    // Cleanup
    sem_destroy(&completed);
    sem_destroy(&mutex);
    return 0;
}

// Sequential FindProduct (no threads)
int SqFindProd(int size) {
    int prod = 1;
    for (int i = 0; i < size; i++) {
        prod *= gData[i];
        prod %= NUM_LIMIT;
        if (gData[i] == 0) break; // Stop if zero is encountered
    }
    return prod;
}

// Thread FindProduct without semaphores
void* ThFindProd(void *param) {
    int threadNum = ((int*)param)[0];
    int startIdx = ((int*)param)[1];
    int endIdx = ((int*)param)[2];
    int prod = 1;

    // Compute the product for the assigned division
    for (int i = startIdx; i <= endIdx; i++) {
        prod *= gData[i];
        prod %= NUM_LIMIT;
        if (gData[i] == 0) {
            gThreadProd[threadNum] = 0;
            gThreadDone[threadNum] = true; // Mark thread as done
            pthread_exit(0); // Exit if zero is found
        }
    }

    gThreadProd[threadNum] = prod;
    gThreadDone[threadNum] = true; // Mark thread as done

    pthread_exit(0);
}

// Thread FindProduct with semaphores
void* ThFindProdWithSemaphore(void *param) {
    int threadNum = ((int*)param)[0];
    int startIdx = ((int*)param)[1];
    int endIdx = ((int*)param)[2];
    int prod = 1;

    // Compute the product for the assigned division
    for (int i = startIdx; i <= endIdx; i++) {
        prod *= gData[i];
        prod %= NUM_LIMIT;
        if (gData[i] == 0) { // If a zero is found, notify parent and exit
            gThreadProd[threadNum] = 0; 
            sem_post(&completed); // Notify parent immediately
            pthread_exit(0); // Exit the thread
        }
    }

    // Store the product for this thread
    gThreadProd[threadNum] = prod;

    // Protect access to gDoneThreadCount with mutex semaphore
    sem_wait(&mutex);
    gDoneThreadCount++;
    if (gDoneThreadCount == gThreadCount) {
        // All threads are done, so signal the parent
        sem_post(&completed);
    }
    sem_post(&mutex);

    pthread_exit(0);
}


// Multiply the division products to compute the total modular product
int ComputeTotalProduct() {
    int i, prod = 1;
    for (i = 0; i < gThreadCount; i++) {
        if (gThreadProd[i] == 0) {
            return 0; // If any thread found a zero, the total product is zero
        }
        prod *= gThreadProd[i];
        prod %= NUM_LIMIT;
    }
    return prod;
}

// Initialize shared variables
void InitSharedVars() {
    int i;
    for (i = 0; i < gThreadCount; i++) {
        gThreadDone[i] = false;
        gThreadProd[i] = 1;
    }
    gDoneThreadCount = 0;
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

// Calculate the right indices to divide the array into thread count equal divisions
void CalculateIndices(int arraySize, int thrdCnt, int indices[MAX_THREADS][3]) {
    int chunkSize = arraySize / thrdCnt;
    for (int i = 0; i < thrdCnt; i++) {
        indices[i][0] = i;
        indices[i][1] = i * chunkSize;
        indices[i][2] = (i == thrdCnt - 1) ? arraySize - 1 : (i + 1) * chunkSize - 1;
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
