#include <stdio.h>
#include <stdlib.h>


#define MEMLENGTH 4096
static int initialized = 0; //A static int to indicate whether the heap has been initialized.


static union{ //Our heap will be this huge array
   char bytes[MEMLENGTH];
   double not_used; //Used for allignment
}heap;


/*
*   This is our chunk. One potion will be our header whcih will be the sizeof(chunk)
*   Total size of the chunk (header + payload)
*   1 if free and 0 if not
*/
typedef struct chunk {
   size_t size; 
   int free;    
}chunk;


/*
*   Leak Detection, will run when program ends and checks how many chunks are not free
*/
static void leakDetection(){
   chunk *ptr = (chunk*) heap.bytes;
   size_t leaks = 0;
   int total = 0;


   while((char *)ptr < heap.bytes + MEMLENGTH){
       if(ptr->free == 0){
           leaks += ptr->size;
           total++;
       }
       ptr = (chunk *)((char *) ptr + ptr->size); //Incrementing over chunks
   }


   if (total > 0) {
       fprintf(stderr, "malloc: %zu bytes leaked in %d objects\n", leaks, total);
   }
}
/*
*   The intialize heap step
*   Before we can allocate any memory we must make our heap one whole chunk. This way we can split the chunk into pieces that can hold our allocated memory
*   We make a global header point to the first byte adress of the heap, then we set our chunks information
*   Initalize heap is now 1
*   The atexit function will be called when the program ends
*/
static void intializeHeap(){
   chunk *first_Chunk = (chunk*) heap.bytes;
   first_Chunk->size = MEMLENGTH;
   first_Chunk->free = 1;
   initialized = 1;
   atexit(leakDetection);
}




/*
*   Below will be our allocation function. A chunk must be free and big enough to store our header and payload, We then must return a void pointer to our payload to the user
*   To determine a chunks size it will be header{sizeof(chunk)} + allignedUserSize
*   Implementation will have two cases
*   Case 1-
*       If the chunk available is able to fit in the users requested chunk size + another minimum chunksize then we can split
*       the chunk into two peices
*       minChunksize = sizeof(chunk) +8 as 8 is the smallest payload option by allignment of payload
*
*   Case 2-
*       If the chunk is free but does not have enough space for another minimum chunksize then we will
*       just mark the whole chunk as free
*/


void * mymalloc(size_t size, char *file, int line){
  
   //First we check if we have intialized our heap yet
   if(initialized!=1){
       intializeHeap();
   }


   if(size == 0){
       return NULL;
   }


   size_t alignedSize = (size + 7) & ~7; //We have to allign the size to be divisible by 8
   size_t totalSize = sizeof(chunk) + alignedSize; //This works cause sizeof(chunk) will always be a multiple of 8 cause of the size_t data type


   if(totalSize > MEMLENGTH){ //If its greater than our total size then we must return an erro
       fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size, file, line);
       return NULL;
   }


   /*
    *   Now that we got the total size lets iterate through our chunks to see who is free and has enough size to hold our total size
    *   Our iteration will use pointer arithmatic so we cast our head to of type char* to increment by one byte each time
    *   heap.bytes+MEMLENGTH, heap.bytes is already of type char so this works
    *  
   */


   chunk *ptr = (chunk*) heap.bytes;
   while((char *)ptr < heap.bytes + MEMLENGTH){ 


       /*
        * First check if our current chunk is free and has enough space for our total size
        */
       if((ptr->free) && (ptr->size >= totalSize)){
           /*
           Now check if our chunk is big enough to split into to two (if it has space of a MinChnuk)
           */
           if((ptr->size >= totalSize +(sizeof(chunk)+8))){


               chunk *newChunk = (chunk *)((char *)ptr + totalSize); //Now chunk points to the beginning of our newChunk
               newChunk->size = ptr->size - totalSize; //The current size of our chunk - the space were making for our alloacted chunk to fit in
               newChunk->free = 1; //Marked as free
              
               ptr->size = totalSize; //The size of our current chunk will change
               ptr->free = 0; //Our current chunk will be the one we send back so make it free


           }else{
               /*
                * If we did not have enough space we just allocate the rest of the chunk. It is the users job not to go over his size which is why this works
               */
               ptr->free = 0;
           }


           //Now we must return the payload in a void ptr which is above the header in my case
           return (void *) ((char *)ptr + sizeof(chunk));
       }


       ptr = (chunk *)((char *) ptr + ptr->size); //Incrementing over chunks
   }


   /*
    *   Now the case where nothing free was found
   */
   fprintf(stderr, "malloc: Unable to allocate %zu bytes (%s:%d)\n", size, file, line);
   return NULL;
}


/*
*   Free-
*   We must make 3 error cases to see if the user has properly entered a valid pointer to our chunk to be marked as free
*   After these errors can calculate our header by paload-sizeof(chunk) and set to free
*   Then we loop through our chunks and coalece adjancent free chunks
*/
void myfree(void *ptr, char *file, int line){


   if (ptr == NULL){
       fprintf(stderr, "free: inappropriate pointer (%s:%d)\n", file, line);
       return;
   }


   /*
    *   Error case 1:
    *   Calling free() with an address not obtained from malloc(), ex int p, free(&p)
    *   This adress will not be within our heap so we check if the adress given is less then our first byte adress of heap or greater than our last byte adress
    *   Any adress not within this heap is not accepeted
   */
   if ((char *)ptr < heap.bytes || (char *)ptr >= heap.bytes + MEMLENGTH) {
       fprintf(stderr, "free: Inappropriate pointer, adress not within bounds of heap (%s:%d)\n", file, line);
       exit(2);
   }


   //Find location of the starting
   chunk *chunkFound = (chunk *)((char*)ptr - sizeof(chunk)); //Cause we start at the payload |header|payload|
 
   /*
    *   Error case 2:
    *   Calling free() with an address not at the start of a chunk, malloc(p+1);
    *   We will iterate throught our chunk adresses and compare with ptr adress
    *   So if ptr != Any Chunk Adress we return an error
   */
   int found = 0;
   chunk *curr = (chunk*) heap.bytes;
   while((char *)curr < heap.bytes + MEMLENGTH){
       if((char *) curr == (char *) chunkFound){ //We cast them to char to compare there one byte location in memory
           found = 1; //There is a match
           break;
       }
       curr = (chunk *)((char *)curr + curr->size);
   }


   if(found == 0){
       fprintf(stderr, "free: Inappropriate pointer, not a correct adress to payload (%s:%d)\n", file, line);
       exit(2);
   }


   /*
    * Case 3: No allowing of double free
    * Since we know that chunkFound is good to use by case 2 we can now use it as regular
   */
   if(chunkFound->free){ //No double free
       fprintf(stderr, "free: Inappropriate pointer, double free detected (%s:%d)\n", file, line);
       exit(2);
   }


   //All error cases are checked now lets free
   chunkFound->free = 1;


   /*
    *  Now we coalesce the whole heap by traversing and merging free adjacent chunks
    */
   curr = (chunk*) heap.bytes;
   chunk *prev = NULL;


   while((char *)curr < heap.bytes + MEMLENGTH){
       if(curr->free && prev != NULL && prev->free){
           prev->size += curr->size; //Merge them
           curr = (chunk *)((char *)prev + prev->size); //Dont move prev but move curr with respect to prev
       }else{
           prev = curr;
           curr = (chunk *)((char *)curr + curr->size);
       }
   }
}









