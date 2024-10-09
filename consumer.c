/*
CSC139 
Fall 2024
First Assignment
Sario, Ryan
Section 01
OSs Tested on: Linux, Mac
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>

#define SHM_SIZE 4096

void* gShmPtr;

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

int main() {
    const char *name = "OS_HW1_ryanSario";
    int bufSize;
    int itemCnt;
    int in;
    int out;

    int shm_fd = shm_open(name, O_RDWR, 0666);
    if (shm_fd == -1) {
        printf("Consumer: Failed to open shared memory block.\n");
        exit(1);
    }

    gShmPtr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (gShmPtr == MAP_FAILED) {
        printf("Consumer: Failed to map shared memory.\n");
        exit(1);
    }

    bufSize = GetBufSize();
    itemCnt = GetItemCnt();
    in = GetIn();
    out = GetOut();

    printf("Consumer reading: bufSize = %d, itemCnt = %d\n", bufSize, itemCnt);

    for (int i = 0; i < itemCnt; i++) {
        while (in == out) {
            in = GetIn(); // Wait until there is something to consume

            // If the producer is done and the buffer is empty, terminate the consumer
            if (GetProducerDone() == 1 && in == out) {
                printf("No more items to consume, exiting.\n");
                exit(0);
            }
        }

        int val = ReadAtBufIndex(out);
        printf("Consuming Item %d with value %d at Index %d\n", i, val, out);

        out = (out + 1) % bufSize; // Update out index
        SetOut(out);
    }

    // Clean up shared memory
    if (shm_unlink(name) == -1) {
        printf("Error removing %s\n", name);
        exit(1);
    }

    return 0;
}

// Check if the producer has completed
int GetProducerDone() {
    return GetHeaderVal(4);  // Assume this flag is at index 4 in shared memory
}


void SetIn(int val) { SetHeaderVal(2, val); }
void SetOut(int val) { SetHeaderVal(3, val); }

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

int GetBufSize() { return GetHeaderVal(0); }
int GetItemCnt() { return GetHeaderVal(1); }
int GetIn() { return GetHeaderVal(2); }
int GetOut() { return GetHeaderVal(3); }

int ReadAtBufIndex(int indx)
{
    void* ptr = gShmPtr + 4 * sizeof(int) + indx * sizeof(int);
    int val;
    memcpy(&val, ptr, sizeof(int));
    return val;
}
