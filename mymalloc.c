#include "mymalloc.h"

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



static char PAGE_TABLE[MEMORY_SIZE / PAGE_SIZE];
static char MAIN_MEMORY[MEMORY_SIZE];


/* ucontext_t struct size is 348 bytes. Assuming max 1000 threads system memory porition for ucontext's of threads
 * must be 348000 bytes. */

int test = 1;

void initializeMemory(){
 
  arrayInitialized = 1;
  char * ptr = MAIN_MEMORY;
  int i = 0;
  for(; i < (MEMORY_SIZE / PAGE_SIZE); i++){
    //printf("Makes it loop %i\n", i);
      * (int *) ptr = (PAGE_SIZE - sizeof(int));
      ptr += PAGE_SIZE;
      //printf("makes it here\n");
  }


  if(0){
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
  short * ptr1 = (MAIN_MEMORY + (401 * PAGE_SIZE)); // Start of page table
  for(; k < 6138; k++){
    *ptr1 = k;
    ptr1++;
  }


  if(0){
    i = 0;
    ptr = MAIN_MEMORY + (401 * PAGE_SIZE);
    for(; i < 6138; i++){
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
  
  	while((tracker + 2 + block_size1)< &myblock[4998]){			// Conditional loop to cycle through all memory blocks in the main memory
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
  			else return;									// the free memory block does nto have a free neighbor 
    		
    	}

 		tracker += (2 + block_size1);
    }
}

/*
 * myFree is a custom replacement method to replace the C standard Library's Free method for freeing dynamocially allocated memory
 

void myfree(void * ptrFree, char * file, int line){
  
  /*	if (!arrayInitialized){
  		*(meta *) myblock = (meta) 4998;
    	arrayInitialized = 1;
    }*
    if(ptrFree == NULL) {
    	printf("Pointer is NULL\n"); 
    	return;
    }

	char * ptrFree1 = (char *) ptrFree;
  
  	if(ptrFree1 < &myblock[2] || ptrFree1>&myblock[4999]){ printf("Pointer ourside of mmemory\n"); return;}

  	char * tracker = myblock;
  	meta meta_size;
  	meta block_size;

  	while(tracker<=(ptrFree1-2)){
  		meta_size = *(meta *)tracker;
  		block_size = *(meta *)tracker - (*(meta *)tracker%2);
   		if(tracker==(ptrFree1-2)){
   			if((*(meta *)tracker%2)==1){
    		*(meta*)tracker = *(meta *)tracker-1;
      		defrag();
      		return;
    		}
    		else{
    			printf("This pointer is to a free block of memory, can not double free\n");
    			return;
    		}
    	}

    	tracker+=(2+block_size);
  }
  printf("Pointer was not an allocated block\n");
  return;

}
*/


void * myallocate(int size, const char* FILENAME, const int LINE) {
	 if (!arrayInitialized){
  	
    	arrayInitialized = 1;

      initializeMemory();
    }

  
  	if (size <= 0){
    	printf("Must be greater than 0\n");
    	return NULL;
  	}
    
   

    int currentBlockSize, initialBlockSize;

     
    if(size > MEMORY_SIZE-sizeof(int)){
      printf("Size bigger then memory");
    }
    

     if(1){
      // find free reserved OS Thread location
      char* ptr = (MAIN_MEMORY + PAGE_SIZE); // First page after Scheduler
      int k = 0;
      for(; k < 200; k++){   // Sear *h through 200 thread contexts  
        if(*(int *) ptr > 0){
          initialBlockSize = abs(*(int *) ptr);
          *(int *) ptr = (size * -1);
          currentBlockSize = abs(*(int *) ptr);
          *(int *) (ptr + sizeof(int) + currentBlockSize) = (initialBlockSize - sizeof(int) - currentBlockSize);
           int pageNum = (ptr - MAIN_MEMORY) / PAGE_SIZE;
           int freePageIndex =  findFreeOSPage();
           *(short *)(MAIN_MEMORY + (401 * PAGE_SIZE) + freePageIndex) = pageNum;
           *(short *)(MAIN_MEMORY + (401 * PAGE_SIZE) + freePageIndex + 1) = pageNum+1;
           return (void *) (ptr + 2); // need to bitmask ptr to shift 13 bits 
        }
        ptr += PAGE_SIZE;
      }



      printf("Max threads reached\n");
    }
    
   /* else{ // Find free Page for Heaps
      meta * ptr = (MEMORY + (400 * PAGE_SIZE) + 1 ); // Point to begining of Page Table
 		
      pthread_init_t currentThread = getThread();
      if(currentThread->heap == NULL){
        int freePageIndex = findFreePage();
        currentThread->heap = (MEMORY + (400 * PAGE_SIZE) + 1 + freePageIndex);
      }
      char * track = currentThread->heap;

      int i = 0;

      while(track < (track + PAGE_SIZE)){
  
        currBlocksize = Math.abs(*(int *)track);
 
        if (*(int *)track > 0 && currBlocksize >= size){  //check if location is free
          *(int *)track = (size * -1)  //negative to signify allocated space 
          *(int *)(track+size+2) = currBlocksize - size -2;
          return (void *) (track+2);
        }
        if(track + *(int *)track + 2 >= track + PAGE_SIZE){
          break;
        }
        
        track += (currBlocksize +2);
      }

      //Makes here wasnt enough room on page, need to get another page.

      int pagesNeeded = (size -*(int *)track) / PAGE_SIZE;
      // No room on this page, need to find free page(s)
      //swapPages(pagesNeeded);
      // After swapping pages

   
      *(int *)track = (size * -1)  //negative to signify allocated space 
      *(int *)(track+size+2) = currBlocksize - size -2;
      return (void *) (track+2); // Need to bitshift trac

    }*/
  
  printf("Not Enough Space\n");
  return NULL;
}

int findFreeOSPage(){
  char * ptr = (MAIN_MEMORY + PAGE_SIZE * 401);
  int i = 0;
  for(; i < 400; i++){ // Firs 400 Page Indexs are for OS Pages
    // Need to pages to allocated a thread
    if(*(short *)ptr > 0 && *(short *)(ptr+2) > 0){
      *(short *) ptr = (*(short *) ptr)* -1;
      *(short *) ptr = (*(short *)ptr +2) * -1;
      return i;
    }
    ptr += 2;
  }

}


