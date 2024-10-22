#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/timeb.h>
#include <semaphore.h>
#include <stdbool.h>

#define MAX_SIZE 100000000
#define MAX_THREADS 16
#define RANDOM_SEED 7649
#define MAX_RANDOM_NUMBER 3000
#define NUM_LIMIT 9973

// Global variables
long gRefTime; // For timing
int gData[MAX_SIZE]; // The array that will hold the data
int gThreadCount; // Number of threads
int gDoneThreadCount; // Threads that are done at a certain point.
int gThreadProd[MAX_THREADS]; // Product for each array division a single thread handles
bool gThreadDone[MAX_THREADS]; // Used when the parent checks on child threads
sem_t completed; // Semaphore to notify parent
sem_t mutex; // Binary semaphore for shared variable gDoneThreadCount

// Function declarations
int SqFindProd(int size);
void *ThFindProd(void *param);
void *ThFindProdWithSemaphore(void *param);
int ComputeTotalProduct();
void InitSharedVars();
void GenerateInput(int size, int indexForZero);
void CalculateIndices(int arraySize, int thrdCnt, int indices[MAX_THREADS][3]);
int GetRand(int min, int max);
long GetMilliSecondTime(struct timeb timeBuf);
long GetCurrentTime(void);
void SetTime(void);
long GetTime(void);

int main(int argc, char *argv[]){
    pthread_t tid[MAX_THREADS];
    pthread_attr_t attr[MAX_THREADS];
    int indices[MAX_THREADS][3];
    int i, indexForZero, arraySize, prod;

    // Parse command-line arguments
    if(argc != 4){
        fprintf(stderr, "Invalid number of arguments!\n");
        exit(-1);
    }
    if((arraySize = atoi(argv[1])) <= 0 || arraySize > MAX_SIZE){
        fprintf(stderr, "Invalid Array Size\n");
        exit(-1);
    }
    gThreadCount = atoi(argv[2]);
    if(gThreadCount > MAX_THREADS || gThreadCount <=0){
        fprintf(stderr, "Invalid Thread Count\n");
        exit(-1);
    }
    indexForZero = atoi(argv[3]);
    if(indexForZero < -1 || indexForZero >= arraySize){
        fprintf(stderr, "Invalid index for zero!\n");
        exit(-1);
    }

    GenerateInput(arraySize, indexForZero);
    CalculateIndices(arraySize, gThreadCount, indices);

    // Sequential calculation
    SetTime();
    prod = SqFindProd(arraySize);
    printf("Sequential multiplication completed in %ld ms. Product = %d\n", GetTime(), prod);

    // Threaded with parent waiting for all children
    InitSharedVars();
    SetTime();
    for (i = 0; i < gThreadCount; i++) {
        pthread_attr_init(&attr[i]);
        pthread_create(&tid[i], &attr[i], ThFindProd, (void *)indices[i]);
    }
    for (i = 0; i < gThreadCount; i++) {
        pthread_join(tid[i], NULL);
    }
    prod = ComputeTotalProduct();
    printf("Threaded multiplication with parent waiting for all children completed in %ld ms. Product = %d\n", GetTime(), prod);

    // Multi-threaded with busy waiting (parent continually checking on child threads without using semaphores)
    InitSharedVars();
    SetTime();
    for (i = 0; i < gThreadCount; i++) {
        pthread_attr_init(&attr[i]);
        pthread_create(&tid[i], &attr[i], ThFindProd, (void *)indices[i]);
    }
    bool allDone = false;
    while (!allDone) {
        allDone = true;
        for (i = 0; i < gThreadCount; i++) {
            if (!gThreadDone[i]) {
                allDone = false;
                break;
            }
        }
    }
    prod = ComputeTotalProduct();
    printf("Threaded multiplication with parent continually checking on children completed in %ld ms. Product = %d\n", GetTime(), prod);

    // Multi-threaded with semaphores
    InitSharedVars();
    sem_init(&completed, 0, 0);
    sem_init(&mutex, 0, 1);
    SetTime();
    for (i = 0; i < gThreadCount; i++) {
        pthread_attr_init(&attr[i]);
        pthread_create(&tid[i], &attr[i], ThFindProdWithSemaphore, (void *)indices[i]);
    }
    sem_wait(&completed);
    for (i = 0; i < gThreadCount; i++) {
        pthread_cancel(tid[i]);
    }
    prod = ComputeTotalProduct();
    printf("Threaded multiplication with parent waiting on a semaphore completed in %ld ms. Prod = %d\n", GetTime(), prod);
    return 0;
}

// Function to sequentially multiply all elements in gData mod NUM_LIMIT
int SqFindProd(int size) {
    int prod = 1;
    for (int i = 0; i < size; i++) {
        if (gData[i] == 0) return 0;
        prod *= gData[i];
        prod %= NUM_LIMIT;
    }
    return prod;
}

// Thread function without semaphores
void *ThFindProd(void *param) {
    int *indices = (int *)param;
    int start = indices[1];
    int end = indices[2];
    int threadNum = indices[0];
    int prod = 1;

    for (int i = start; i <= end; i++) {
        if (gData[i] == 0) {
            gThreadDone[threadNum] = true;
            pthread_exit(0);
        }
        prod *= gData[i];
        prod %= NUM_LIMIT;
    }

    gThreadProd[threadNum] = prod;
    gThreadDone[threadNum] = true;
    pthread_exit(0);
}

// Thread function with semaphores
void *ThFindProdWithSemaphore(void *param) {
    int *indices = (int *)param;
    int start = indices[1];
    int end = indices[2];
    int threadNum = indices[0];
    int prod = 1;

    for (int i = start; i <= end; i++) {
        if (gData[i] == 0) {
            sem_post(&completed);
            pthread_exit(0);
        }
        prod *= gData[i];
        prod %= NUM_LIMIT;
    }

    gThreadProd[threadNum] = prod;
    sem_wait(&mutex);
    gDoneThreadCount++;
    if (gDoneThreadCount == gThreadCount) {
        sem_post(&completed);
    }
    sem_post(&mutex);
    pthread_exit(0);
}

// Compute the final modular product
int ComputeTotalProduct() {
    int prod = 1;
    for (int i = 0; i < gThreadCount; i++) {
        prod *= gThreadProd[i];
        prod %= NUM_LIMIT;
    }
    return prod;
}

// Initialize shared variables
void InitSharedVars() {
    for (int i = 0; i < gThreadCount; i++) {
        gThreadDone[i] = false;
        gThreadProd[i] = 1;
    }
    gDoneThreadCount = 0;
}

// Generate the input array
void GenerateInput(int size, int indexForZero) {
    srand(RANDOM_SEED);
    for (int i = 0; i < size; i++) {
        gData[i] = GetRand(1, MAX_RANDOM_NUMBER);
    }
    if (indexForZero >= 0) {
        gData[indexForZero] = 0;
    }
}

// Calculate indices to divide the array into divisions
void CalculateIndices(int arraySize, int thrdCnt, int indices[MAX_THREADS][3]) {
    int chunkSize = arraySize / thrdCnt;
    for (int i = 0; i < thrdCnt; i++) {
        indices[i][0] = i;
        indices[i][1] = i * chunkSize;
        indices[i][2] = (i == thrdCnt - 1) ? arraySize - 1 : (i + 1) * chunkSize - 1;
    }
}

// Get a random number between x and y
int GetRand(int x, int y) {
    return x + rand() % (y - x + 1);
}

// Timing functions
long GetMilliSecondTime(struct timeb timeBuf){
    long milliSecondTime;
    milliSecondTime = timeBuf.time;
    milliSecondTime *= 1000;
    milliSecondTime += timeBuf.millitm;
    return milliSecondTime;
}

long GetCurrentTime(void){
    long crntTime = 0;
    struct timeb timeBuf;
    ftime(&timeBuf);
    crntTime = GetMilliSecondTime(timeBuf);
    return crntTime;
}

void SetTime(void){
    gRefTime = GetCurrentTime();
}

long GetTime(void){
    long crntTime = GetCurrentTime();
    return (crntTime - gRefTime);
}
