/*
CSC139 
Fall 2024
First Assignment
Sario, Ryan
Section 01
OSs Tested on: such as Linux, Mac, etc.
*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>

// Size of shared memory block
// Pass this to ftruncate and mmap
#define SHM_SIZE 4096

// Global pointer to the shared memory block
// This should receive the return value of mmap
// Don't change this pointer in any function
void* gShmPtr;

// You won't necessarily need all the functions below
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

int main()
{
    const char *name = "OS_HW1_ryanSario"; // Name of shared memory block to be passed to shm_open
    int bufSize; // Bounded buffer size
    int itemCnt; // Number of items to be consumed
    int in; // Index of next item to produce
    int out; // Index of next item to consume

    // Create and map the shared memory block
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
     
     // Write code here to create a shared memory block and map it to gShmPtr
     // Use the above name
     // **Extremely Important: map the shared memory block for both reading and writing 
     // Use PROT_READ | PROT_WRITE



     // Write code here to read the four integers from the header of the shared memory block 
     // These are: bufSize, itemCnt, in, out
     // Just call the functions provided below like this:
     bufSize = GetBufSize();
     itemCnt = GetItemCnt();
     in = GetIn();
     out = GetOut();
	
     // Check that the consumer has read the right values
    printf("Consumer reading: bufSize = %d, itemCnt = %d\n", bufSize, itemCnt);

    // Consume all the items produced by the producer
    for (int i = 0; i < itemCnt; i++) {
        // Wait if buffer is empty (i.e., in == out)
        while (in == out) {
            in = GetIn();  // Update 'in' index from shared memory
        }

    // Read item at 'out' index from the bounded buffer
        int val = ReadAtBufIndex(out);

        // Report the consumption of the item
        printf("Consuming Item %d with value %d at Index %d\n", i, val, out);

        // Update the "out" index, wrapping around the buffer size
        out = (out + 1) % bufSize;

        // Set the updated "out" value in shared memory
        SetOut(out);
    }
          
     // remove the shared memory segment 
     if (shm_unlink(name) == -1) {
	printf("Error removing %s\n",name);
	exit(-1);
     }

     return 0;
}


// Set the value of shared variable "in"
void SetIn(int val)
{
        SetHeaderVal(2, val);
}

// Set the value of shared variable "out"
void SetOut(int val)
{
        SetHeaderVal(3, val);
}

// Get the ith value in the header
int GetHeaderVal(int i)
{
        int val;
        void* ptr = gShmPtr + i*sizeof(int);
        memcpy(&val, ptr, sizeof(int));
        return val;
}

// Set the ith value in the header
void SetHeaderVal(int i, int val)
{
    void* ptr = gShmPtr + i * sizeof(int);
    memcpy(ptr, &val, sizeof(int));
}

// Get the value of shared variable "bufSize"
int GetBufSize()
{       
        return GetHeaderVal(0);
}

// Get the value of shared variable "itemCnt"
int GetItemCnt()
{
        return GetHeaderVal(1);
}

// Get the value of shared variable "in"
int GetIn()
{
        return GetHeaderVal(2);
}

// Get the value of shared variable "out"
int GetOut()
{             
        return GetHeaderVal(3);
}


// Write the given val at the given index in the bounded buffer 
void WriteAtBufIndex(int indx, int val)
{
    void* ptr = gShmPtr + 4 * sizeof(int) + indx * sizeof(int);
    memcpy(ptr, &val, sizeof(int));

}

// Read the val at the given index in the bounded buffer
int ReadAtBufIndex(int indx)
{
    void* ptr = gShmPtr + 4 * sizeof(int) + indx * sizeof(int);
    int val;
    memcpy(&val, ptr, sizeof(int));
    return val;
 
}

