#if defined(USE_MY_MALLOC)
#include <stdlib.h>
#include <stdio.h>
#include <sys/mman.h>
#include "mymalloc2.h"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"

// total memory available == 1 * 2^20...
//#define memsize 1<<20
//static const size_t MEMSIZE = 5000;
static const size_t FREE_SIG = 0xAAAAAAAA;
static const size_t USED_SIG = 0xAAAAAAAB;
#define ALIGN8(a) (((a)+7)&~7)
// our "virtual" memory to be managed by custom malloc/free

void my_malloc2_init(void ** mem_pool, size_t size, int protection, void * addr){
  struct MemEntry * root = (struct MemEntry*)mmap(addr, size, protection /*PROT_WRITE | PROT_READ | PROT_EXEC*/, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  *mem_pool = root;
  root->prev = root->succ = NULL;
  root->size = size - ALIGN8(sizeof(MemEntry));
  root->recognize = FREE_SIG;
}

void my_malloc2_init2(void * mem_pool, size_t size){
  struct MemEntry * root = (struct MemEntry*)mem_pool;
  root->prev = root->succ = NULL;
  root->size = size - ALIGN8(sizeof(MemEntry));
  root->recognize = FREE_SIG;
}

/*
  Algorithm:
  traverse from left to right in mem_space and find MemEntry which is marked as free
  check size of this memEntry
  if memEntry.size < size:
    keep traversing
  else
    allocate (mark this memEntry as allocated)
      check memEntry.size
      if memEntry.size > size && memEntry.size - size > offsetValue:
        create a new MemEntry following end of newly allocated block
    return address to beginning of data block following this memEntry
*/
void *mymalloc2(void * mem_pool, size_t size, const char * file, int line){
  struct MemEntry * root = (struct MemEntry*)mem_pool;
  struct MemEntry * p, *succ; //todo: change name of succ

  // *** algorithm for allocated chunks of memory ***
  p = root;
  size = ALIGN8(size);
  do{
    if(p->size < size) p = p->succ; // p is not big enough for allocation
    else if(p->recognize != FREE_SIG) p = p->succ; // p is not free to be allocated
    else if(p->size >= size) // found a chunk large enough
    {
      p->recognize = USED_SIG;
      // break off current data-block & make another MemEntry struct
      if(p->size > (size + ALIGN8(sizeof(MemEntry)))){
        succ = (MemEntry*)((size_t)p + ALIGN8(sizeof(MemEntry)) + size);
        succ->prev = p;
        succ->succ = p->succ;
        succ->size = p->size - ALIGN8(sizeof(MemEntry)) - size;
        succ->recognize = FREE_SIG;
        p->size = size;
        p->succ = succ;
      }
      return (void*)((size_t)p + ALIGN8(sizeof(MemEntry)));
    }
  } while(p);
  //we ran out of space
  printf(ANSI_COLOR_RED "malloc() called failed from %s, line %d\n\tError: Not enough contiguous memory. Try freeing first.\n" ANSI_COLOR_RESET, file, line);
  return NULL;
}

/*
  Algorithm:
    Check if address is in the mem_space array
      if yes:
        check if address points to the beginning of a data-block (end of a MemEntry).
        To do this: check if the value before it matches the recognize pattern
          size_t recognize == SIGNATURE
          if this matches: go to beginning of MemEntry & mark the block as free
            isFree = 1
          check if successor pointer is to a free MemEntry
            if yes:
              merge the MemEntrys (size of current MemEntry = MemEntry.size + ALIGN8(sizeof(MemEntry)) + MemEntry.succ.size)
              set succ pointer to the successor of the second MemEntry and then check the successor of that MemEntry
          Keep repeating until you encounter a MemEntry that is NOT free (isFree = 0)

          check if prev pointer is to a free MemEntry
            if yes:
              merge current MemEntry with prev MemEntry
              set the succ pointer of prev MemEntry to the succ of current MemEntry
              set current MemEntry to be the old prev MemEntry
            keep repeating until you encounter a MemEntry that is NOT free (isFree = 0)
*/
void myfree2(void * mem_pool, void *address, const char * file, int line){
  struct MemEntry *p;
  struct MemEntry * root = (struct MemEntry*)mem_pool;
  // if(address > (void*)root && address < (void*)((char*)root + root->size + 1)){
    if(*(size_t*)(address - sizeof(size_t)) == USED_SIG){
      p = (MemEntry*)((size_t)address - ALIGN8(sizeof(MemEntry)));
    }
    else{
      printf(ANSI_COLOR_RED "free() call failed from %s, line %d\n\tError: Attempted to free nonexistent/already-freed memory.\n" ANSI_COLOR_RESET, file, line);
      return; //address not in memory
    }
    p->recognize = FREE_SIG;
  // }
  // else{
  //   printf(ANSI_COLOR_RED "free() call failed from %s, line %d\n\tError: Attempted to free an array not in allocatable memory.\n" ANSI_COLOR_RESET, file, line);
  //   printf("Mempool: %p, Address: %p, Root size: %zu\n", mem_pool, address, root->size);
  //   return;
  // }
  //merge free data blocks into one free data block
  while(p->prev != NULL && p->prev->recognize == FREE_SIG){
    p = p->prev;
  } //keep finding prev pointer until no longer free, now merge all free blocks before and after
  while(p->succ != NULL && p->succ->recognize == FREE_SIG){ //merge with additional block
    p->size += p->succ->size + ALIGN8(sizeof(MemEntry));
    p->succ->recognize = 0; //to avoid getting picked up by future addresses
    p->succ = p->succ->succ;
  }
}
#endif
