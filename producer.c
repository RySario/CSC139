/*
CSC139 
Fall 2024
First Assignment
Sario, Ryan
Section 01
OSs Tested on: Mac
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/types.h>

// Size of shared memory block
#define SHM_SIZE 4096

// Global pointer to the shared memory block
void* gShmPtr;

void Producer(int, int, int);
void InitShm(int, int);
void SetBufSize(int);
void SetItemCnt(int);
void SetIn(int);
void SetOut(int);
void SetHeaderVal(int, int);
int GetBufSize();
int GetItemCnt();
int GetIn();
int GetOut();
int GetHeaderVal(int);
void WriteAtBufIndex(int, int);
int ReadAtBufIndex(int);
int GetRand(int, int);

int main(int argc, char* argv[])
{
    pid_t pid;
    int bufSize; 
    int itemCnt; 
    int randSeed;

    if (argc != 4) {
        printf("Invalid number of command-line arguments\n");
        exit(1);
    }

    bufSize = atoi(argv[1]);
    itemCnt = atoi(argv[2]);
    randSeed = atoi(argv[3]);

    if (bufSize < 2 || bufSize > 480) {
        printf("Invalid buffer size. Must be between 2 and 480.\n");
        exit(1);
    }

    if (itemCnt <= 0) {
        printf("Invalid item count. Must be greater than 0.\n");
        exit(1);
    }

    srand(randSeed);
    InitShm(bufSize, itemCnt);

    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "Fork Failed\n");
        exit(1);
    } else if (pid == 0) {
        printf("Launching Consumer \n");
        execlp("./consumer", "consumer", NULL);
    } else {
        printf("Starting Producer\n");
        Producer(bufSize, itemCnt, randSeed);
        printf("Producer done and waiting for consumer\n");
        wait(NULL);
        printf("Consumer Completed\n");
    }

    return 0;
}

void InitShm(int bufSize, int itemCnt)
{
    int in = 0;
    int out = 0;
    const char *name = "OS_HW1_ryanSario";

    int shm_fd = shm_open(name, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        printf("Failed to create shared memory block.\n");
        exit(1);
    }

    ftruncate(shm_fd, SHM_SIZE);
    gShmPtr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (gShmPtr == MAP_FAILED) {
        printf("Failed to map shared memory.\n");
        exit(1);
    }

    SetBufSize(bufSize);
    SetItemCnt(itemCnt);
    SetIn(in);
    SetOut(out);
}

void Producer(int bufSize, int itemCnt, int randSeed) {
    int in = 0;
    int out = 0;

    srand(randSeed);
    in = GetIn();
    out = GetOut();

    for (int i = 0; i < itemCnt; i++) {
        while (((in + 1) % bufSize) == out) {
            out = GetOut(); // Wait until space is available
        }

        int val = GetRand(2, 5200);
        WriteAtBufIndex(in, val);
        printf("Producing Item %d with value %d at Index %d\n", i, val, in);

        in = (in + 1) % bufSize; // Update in index
        SetIn(in);
    }

    // Set the "producer done" flag in shared memory
    SetProducerDone(1);
    printf("Producer Completed\n");
}

// Set the producer done flag
void SetProducerDone(int val) {
    SetHeaderVal(4, val);  // Assume this flag is at index 4 in shared memory
}


void SetBufSize(int val) { SetHeaderVal(0, val); }
void SetItemCnt(int val) { SetHeaderVal(1, val); }
void SetIn(int val) { SetHeaderVal(2, val); }
void SetOut(int val) { SetHeaderVal(3, val); }
int GetBufSize() { return GetHeaderVal(0); }
int GetItemCnt() { return GetHeaderVal(1); }
int GetIn() { return GetHeaderVal(2); }
int GetOut() { return GetHeaderVal(3); }

int GetHeaderVal(int i)
{
    int val;
    void* ptr = gShmPtr + i * sizeof(int);
    memcpy(&val, ptr, sizeof(int));
    return val;
}

void SetHeaderVal(int i, int val)
{
    void* ptr = gShmPtr + i * sizeof(int);
    memcpy(ptr, &val, sizeof(int));
}

void WriteAtBufIndex(int indx, int val)
{
    void* ptr = gShmPtr + 4 * sizeof(int) + indx * sizeof(int);
    memcpy(ptr, &val, sizeof(int));
}

int GetRand(int x, int y)
{
    int r = rand();
    r = x + r % (y - x + 1);
    return r;
}
