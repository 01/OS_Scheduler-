#include "mymalloc.h"
#include "my_pthread_t.h"
#include <stdlib.h> //for memalign
#include <sys/mman.h>


int arrayInitialized=0;

// Things to include in Mem Manager, or Scheduler Struct
// 1.) Pointer to begining of PageTable
// 2.) Pointer to begining of heaps

/* Memory for main Memory and abstraction for page tables being contiguous
 *
 *  8MB is 8,388,608 bytes
 *
 *  Main_Memory - Ucontext space = 8040607 Bytes / (4096 (PAGE_SIZE) + 2 (MetaData Size) = 1963ish Pages
 *
 *  2000 indexable pages requires 11 bits (will need to use 2 bytes)
 *
/* Physical Memory Diagram ************************************
 *----------------------------------------------------------  *
 *- ucontext's   -  Page Table      -    Physical Memory   -  *                                     -
 *- 348000Bytes  -   4000 bytes     -     8,036,608 bytes  -  *
 *----------------------------------------------------------  *
 **************************************************************
 *
 * VM Address Diagram*******************************************
 * ----------------------------------------------------------- *
 * - Page Index   -                 Meta Data                - *
   -              -------------------------------------------- *
 * - 16 bits      -  Contiguous/Not -  Free/Not  - Block Size- *
 * -              -     1 but       -   1 bit    -   14 bits - *
 * ----------------------------------------------------------- *
 ***************************************************************
*/

// Meta Data 4 byte int. 3 bytes size (negative allocated), 4th byte threadID


// TODO: Make sure this is memaligned!!
static char * MAIN_MEMORY;

memManager * manager;


/* ucontext_t struct size is 348 bytes. Assuming max 1000 threads system memory porition for ucontext's of threads
 * must be 348000 bytes. */

int test = 0;


// Set up the main memory space for OS + reserved space + heap
// TODO: What is the OS Region vs. what is the Heap Page Table?
//       Should the global page table be within the OS region?
void initializeMemory(){

  arrayInitialized = 1;
  MAIN_MEMORY = (char*)memalign(PAGE_SIZE, MEMORY_SIZE);
  char * ptr = MAIN_MEMORY;

  // insert metadata at begining of each page block
  int i = 0;
  for(; i < numPages; i++){
    // sizeof(meta data) == sizeof(int)
    *(int *) ptr = (PAGE_SIZE - sizeof(int));
    ptr += PAGE_SIZE;
  }

  // Save ptr to each key memory area in main memory
  // TODO:  Create definition for memManager struct
  //memManager manager;
  //* (memManager *) (MAIN_MEMORY + 2) = manager;

  //memManager * manager1 = (MAIN_MEMORY + 2);
  manager = MAIN_MEMORY;
  manager->OS_Region = (MAIN_MEMORY + PAGE_SIZE);
  manager->Reserved_Page_Table = (MAIN_MEMORY + (401 *PAGE_SIZE));
  manager->Heap_Page_Table = manager->Reserved_Page_Table + (200 * sizeof(short));

  if(test){
    i = 0;
    ptr = MAIN_MEMORY;
    for(; i < (MEMORY_SIZE/PAGE_SIZE); i++){
      printf("Page %i : Metadata = %i", i, *(int *)ptr);
      ptr+=PAGE_SIZE;
    }
  }

  // Scheduler gets one page
  // *(scheduler *) MEMORY[0]= main;

  // Initialize reserved OS for 200 Threads 2 Pages per 400 pages

  // Initialize Page Table, index ~6139 Pages (1775 main memory 4096 Swap), 2 Bytes each for Frame offset 12278 bytes

  // First 401 Pages Reserved
  int k = 0;
  short * ptr1 = manager->Reserved_Page_Table; // Start of page table
  for(; k < numPages; k++){
    // TODO: is this bitShift(...) an actual function?
    *ptr1 = bitShift(k, 8);
    ptr1++;
  }

  if(test){
    i = 0;
    ptr =  manager->Reserved_Page_Table;
    for(; i < numPages; i++){
      printf("Page Index %i : Metadata = %i\n", i, *(short *)ptr);
      ptr+=2;
    }
  }
}


/*
 * The Defrag method is a method called each time a pointer is successfully freed. its purpose is to combine neighboring free memory blocks into a single memory
 * blocks. The method checks a memory block to see if it is free or allocated. If it is free, a loop begins combining the consecective free memory blocks until
 * neighbor memory block is not a free memory block This defrag method takes in no inputs, and assumess access to the memory it is going to defrag.
 * Though be more efficient to end the method once two free neighboring memory blocks are combined and posssibly a 3rd, since in this implementation
 * there is no scenario where more then 3 consequtive memory blocks are all free, the method continues through the whole memory so that this method can
 * defrag an array
*/
/*void defrag(){
  char * tracker = myblock;
    meta meta_size1 = * (meta *) tracker;
    meta block_size1 = (meta_size1 - (meta_size1%2));
    meta block_size2 =0;
    meta meta_size2 = 0;
    if((tracker + 2 + block_size1)<&myblock[4998]){
      meta_size2 = * (meta *) (tracker +2 + block_size1);
      block_size2 = (meta_size2 - (meta_size2%2));
    }
    else {return;}

    while((tracker + 2 + block_size1)< &myblock[4998]){      // Conditional loop to cycle through all memory blocks in the main memory
      meta_size1 = * (meta *) tracker;
      block_size1 = (meta_size1 - (meta_size1%2));
      if((tracker + 2 + block_size1)<&myblock[4997]){
      meta_size2 = * (meta *) (tracker +2 + block_size1);
      block_size2 = (meta_size2 - (meta_size2%2));
    }
    else return;

      while((meta_size1%2==0) && (meta_size2%2==0) && (tracker + 2 + block_size1)< &myblock[4997]){ // Conditional loop to continue combining neighboring free memory blocks until

        *(meta *)tracker += (block_size2 + 2);
        return;
        meta_size1 = * (meta *) tracker;
        block_size1 = (meta_size1 - (meta_size1%2));
        if((tracker + 2 + block_size1)<&myblock[4998]){
          meta_size2 = * (meta *) (tracker +2 + block_size1);
          block_size2 = (meta_size2 - (meta_size2%2));
        }
        else return;                  // the free memory block does nto have a free neighbor

      }

     tracker += (2 + block_size1);
    }
}

/*
 * myFree is a custom replacement method to replace the C standard Library's Free method for freeing dynamocially allocated memory
*/

void mydeallocate(void * ptrFree, const char* FILENAME, const int LINE, int caller){

  if (!arrayInitialized){
    initializeMemory();
    arrayInitialized = 1;
  }
  if(ptrFree == NULL){
    printf("Pointer is NULL\n");
    return;
  }

  char * ptrFree1 = (char *) ptrFree;

  if(ptrFree1 < MAIN_MEMORY || ptrFree1 > (MAIN_MEMORY + MEMORY_SIZE + SWAP_SIZE)){
    printf("Pointer ourside of mmemory\n");
    return;
  }

  int pageIndex = bitShift(ptrFree1, -16);
  int offset = bitShift(ptrFree1, 16);
  int meta_size, block_size;
  int frameNumber = *(short *)(manager->Reserved_Page_Table + (pageIndex * 2));
  char * tracker = manager->user_heap + (frameNumber * PAGE_SIZE);

  while(tracker<=(ptrFree1 + PAGE_SIZE)){
    block_size = abs(*(int *)tracker);

    if(tracker==(ptrFree1-2)){
      if((*(int *)tracker < 0)){
        *(int *)tracker = abs(*(int *)tracker);
        defrag();
        return;
      }
      else{
        printf("This pointer is a free block of memory, can not double free\n");
        return;
      }
    }

    tracker+=(2+block_size);
  }
  printf("Pointer was not an allocated block\n");
  return;
}

int findFreeOSPage(){
  // TODO: Revise to actually retrieve the Reserved_Page_Table ptr value
  char * ptr = manager->Reserved_Page_Table;
  int i = 0;
  for(; i < 400; i++){ // First 400 Page Indexs are for OS Pages
    // Need to pages to allocated a thread
    if(*(short *)ptr > 0 && *(short *)(ptr+2) > 0){
      *(short *) ptr = (*(short *) ptr)* -1;
      *(short *) ptr = (*(short *)ptr +2) * -1;
      return i;
    }
    ptr += 2;
  }

}

int findFreeHeapPage(){
  // TODO: Revise to actually retrieve the Heap_Page_Table ptr value
  char * ptr = manager->Heap_Page_Table;
  int i = 0;
  // TODO:  numPages is undefined in this scope
  //        Not sure where numPages is coming from...
  for(; i < (numPages *2)-400; i++){ // Search pages after the OS reserved Pages
    if(*(short *)ptr > 0 && (*(short *)ptr < MAIN_MEMORY + MEMORY_SIZE)){
      *(short *) ptr = (*(short *) ptr)* -1;
      return i;
    }
    ptr += 2;
  }
}

void * libraryAllocate(unsigned int size){
  char* ptr = (MAIN_MEMORY + PAGE_SIZE); // First page after Scheduler
  int k = 0;
  for(; k < 200; k++){   // Sear *h through 200 thread contexts
    if(*(int *) ptr > 0){
      int initialBlockSize = abs(*(int *) ptr);
      *(int *) ptr = (size * -1);
      int currentBlockSize = abs(*(int *) ptr);
      *(int *) (ptr + sizeof(int) + currentBlockSize) = (initialBlockSize - sizeof(int) - currentBlockSize);
      int pageNum = (ptr - MAIN_MEMORY) / PAGE_SIZE;
      int freePageIndex =  findFreeOSPage();
      *(short *)(manager->Reserved_Page_Table + (freePageIndex *2)) = pageNum * -1;
      *(short *)(manager->Reserved_Page_Table + (freePageIndex *2) + 1) = -1 * (pageNum+1);
      return (void *) (ptr + 2); // need to bitmask ptr to shift 13 bits
    }
    ptr += PAGE_SIZE;
  }

  printf("Max threads reached\n");
  return NULL;
}

void * threadAllocate(unsigned int size){
  int * ptr = manager->Reserved_Page_Table; // Point to begining of Page Table

      my_pthread_int_t * currentThread = __current_thread;
      if(currentThread->heap == NULL){
        int freePageIndex = findFreeHeapPage();
        if(freePageIndex < 0){
          // need to swap
        }
        currentThread->heap = (manager->Reserved_Page_Table + (freePageIndex * 2));
      }
      char * track = currentThread->heap;

      int i = 0;

      //TODO: edit everything involving threadID in some way (heap is now a thing though)

      if(bitShift(*(int *) track, -16) != threadID){
        // pageFault find where page went
      }

      while(track < (track + PAGE_SIZE)){

        int currentBlocksize = bitShift(abs(*(int *)track), 8);

        if (*(int *)track > 0 && currentBlocksize >= size){  //check if location is free
          //leading byte of Page contains threadID maybe.....
          *(int *)track = bitShift(size, 8) + threadID  //negative to signify allocated space
          *(int *)(track+size+2) = bitShift((currentBlocksize - size -2), 8) + threadID;
          return (void *) bitShift((((track+2 - (char*)(currentThread->heap))/PAGE_SIZE) + currentThread->heap), 16) + (track+2 - (char*)(currentThread->heap));
        }
        if((track + currentBlocksize + 2) == (track + PAGE_SIZE){
          break;
        }

        track += (currentBlocksize +2);
      }

      //Makes here wasnt enough room on page, need to get another page.

      int pagesNeeded = (size -*(int *)track) / PAGE_SIZE;
     // Manager will need to update table and associate with thread heap,

      if(numFreePages() < pagesNeeded){
        if(freePage == NULL){
          // swap out from swapFile;
          if(swapPage == NULL){
            int numFreeSwaps = numFreeSwaps();
            if(numFreeSwaps < pagesNeeded);
            printf("All memory is full including Swap files\n");
          }
        }
      }
      else{
        // get free pages in main memory and swap them into contigous place
        int pageLocationNeeded = (track - (char*)(currentThread->heap));
      }

      // No room on this page, need to find free page(s)
      //swapPages(pagesNeeded);
      // After swapping pages


  //}

  //printf("Not Enough Space\n");
  //return NULL;

}

void * myallocate(unsigned int size, const char* FILENAME, const int LINE, int caller) {

  // If memory not initialized, initialize it
  // TODO: Initialize scheduler and memory manager
  if (!arrayInitialized){
    arrayInitialized = 1;
    initializeMemory();
  }

  int currentBlockSize, initialBlockSize;

  if(size > MEMORY_SIZE-sizeof(int)){
    printf("Size bigger then memory");
  }

  if(caller == LIBRARYREQ){
    // find free reserved OS Thread location
    return allocateThread(size);
  }
  else { // Find free Page for Heaps
    return threadAllocate(size);
  }
}

void swap_pages(void * page_out, void * page_in) {
  mprotect(page_out, PAGE_SIZE, PROT_READ|PROT_WRITE);

  int page_in_id = bitShift(*(int *)page_in, -8);
  int page_out_id = bitShift(*(int *)page_out, -8);

  memcpy(manager->swap/* what is this doing? */, page_out, PAGE_SIZE);
  memcpy(page_in,page_out, PAGE_SIZE);
  memcpy(page_out, page_in, PAGE_SIZE);


  // Need to find way to update Page_Table

  mprotect(page_in, PAGE_SIZE, PROT_NONE);
}

int bitShift(int value, int shift){
  return (shift > 0) ? (value << shift) : (value >> shift);
}

void * findFreeSwap(){
  char * ptr = manager->swap_file;
  int i = 0;
  for(; i < Swap_Pages; i++){
    if(*(int *) ptr > 0){
        return ptr;
    }
    ptr += PAGE_SIZE;
  }

  return NULL;
}

int numFreeSwaps(){
  char * ptr = manager->swap_file;
  int i = 0;
  int freeSwaps = 0;
  for(; i < Swap_Pages; i++){
    if(*(int *) ptr > 0){
        freeSwaps++;
    }
    ptr += PAGE_SIZE;
  }

  return freeSwaps;

}

int numFreePages(){
   char * ptr = manager->user_heap;

   int i = 0;
  int freePages = 0;
  for(; i < (MEMORY_SIZE/PAGE_SIZE); i++){
    if(*(int *) ptr > 0){
        freeSwaps++;
    }
    ptr += PAGE_SIZE;
  }

  return freeSwaps;

}