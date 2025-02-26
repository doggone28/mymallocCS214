#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "mymalloc.h" // Include your memory allocator header

#define HEADERSIZE 16   
#define NUM_RUNS 50
#define NUM_ALLOCS 120


void test1() {
    for (int i = 0; i < NUM_ALLOCS; i++) {
        void *ptr = malloc(1);
   //     printf("Allocated one byte!\n");
        if (ptr) {
            free(ptr);
          //  printf("Freed one byte!\n");
        }
    }
}


int test2(){


    void *ptrs[NUM_ALLOCS];
    printf("Starting allocation of arrays!\n");
    for(int i = 0; i < NUM_ALLOCS; i++){
        ptrs[i] = malloc(1);
        if(!ptrs[i]){
            printf("Memory allocation failed at index %d!\n", i);
            return 1;
        }
    }


    printf("Allocation complete! Starting to free \n");


    for(int i = 0; i < NUM_ALLOCS; i++){
        free(ptrs[i]);
    }


    printf("Everything done! \n");


    return 0;
}



void test3() {
    void *ptrs[NUM_ALLOCS] = {NULL}; // Array of 120 pointers, initialized to NULL
    int allocated = 0; // Counter for number of allocations

    int freed = 0;


    while (allocated < NUM_ALLOCS) {
        int choice = rand() % 2; // Randomly choose between malloc and free

        if (choice == 0) { // Allocate
            for (int i = 0; i < NUM_ALLOCS; i++) {
                if (ptrs[i] == NULL) { // Find an empty spot
                    ptrs[i] = malloc(1);
                    if (ptrs[i] != NULL) {
                        allocated++;
                        
                    }
                    break;
                }
            }
        } else if(allocated > freed){ // Free a previously allocated object
            for (int i = 0; i < NUM_ALLOCS; i++) {
                if (ptrs[i] != NULL) { // Find an allocated object
                    free(ptrs[i]);
                    ptrs[i] = NULL;
                    freed++;
                    break;
                }
            }
        } else{
            break;
        }
    }

    // Ensure all allocations are freed
    for (int i = 0; i < NUM_ALLOCS; i++) {
        if (ptrs[i] != NULL) {
            free(ptrs[i]);
            ptrs[i] = NULL;
        }
    }
}







int main() {
    struct timeval start, end;
    double total_time = 0;


    printf("Starting memgrind workload...\n");


    for (int i = 0; i < NUM_RUNS; i++) {
        gettimeofday(&start, NULL); // Get start time
       test1(); // Execute the workload


       test2();
       test3();
       printf("Made it here!");
        gettimeofday(&end, NULL); // Get end time
          double elapsed_time = (end.tv_sec - start.tv_sec) * 1000000L + (end.tv_usec - start.tv_usec);
        total_time += elapsed_time;
        printf("Run %d completed. End time: %ld seconds, %ld microseconds\n",
               i + 1, end.tv_sec, end.tv_usec);
    }


    printf("Completed %d runs.\n", NUM_RUNS);
    printf("Average execution time: %lf microseconds per run.\n", total_time / NUM_RUNS);


    return 0;
}







