#ifndef __MEMORYALLOC_H__
#define __MEMORYALLOC_H__
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h> // for memalign
#include <sys/mman.h> // for mprotect
#include <string.h> // for memset
#include <unistd.h>

#include <errno.h> // for debugging errors from mprotect
#include <signal.h>

#include <ctype.h>
#include <time.h>

//#define extern char *error = "Not Enough Space";
#define malloc(x) myallocate(x, __FILE__, __LINE__, THREADREQ);
#define free(x) mydeallocate(x, __FILE__, __LINE__, THREADREQ);
#define USER_POOL_SIZE (1024*1024*7)
#define SYSTEM_POOL_SIZE (1024*1024)

#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)
#define MEMORY_SIZE 8 * 1024 * 1024
#define SCHEDULER_SIZE 1
#define THREAD_RESERVED_SIZE 400
// store the page tables for the 200 threads here
// this is to keep track of which pages the thread owns & what loc to load to for swap
#define THREAD_TABLE_PAGES 1
// global page table to keep track of where each page each
#define GLOBAL_PT_SIZE 3
#define HEAP_SLOT_COUNT (MEMORY_SIZE - PAGE_SIZE * (SCHEDULER_SIZE + THREAD_RESERVED_SIZE + THREAD_TABLE_PAGES + GLOBAL_PT_SIZE))/PAGE_SIZE

#define SWAP_SIZE 16 * 1024 * 1024
#define UCONTEXT_SIZE sizeof(ucontext_t);
// #define NUM_PAGES ((16 + 8) * 1024 * 1024)/PAGE_SIZE
// FIXME: when swapfile added, add SWAP_SLOT_COUNT to NUM_PAGES
#define NUM_PAGES (HEAP_SLOT_COUNT + SWAP_SLOT_COUNT) 

#define ERROR_NOT_ALLOCATED printf("ERROR: Attempted to free an unallocated block. Line: %d File: %s\n", __LINE__, __FILE__);
#define ERROR_NOT_ENOUGH_SPACE printf("ERROR: Attempting to malloc too much space. Line: %d File: %s\n", __LINE__, __FILE__);
#define THREADREQ 1

// typedef struct memManager{
//  void * OS_Region;
//  void * Reserved_Page_Table;
//  void * Heap_Page_Table;
//  void * user_heap; //needs to be initalized
//  void * page_table; //needs to be initialized
//  void * swap_file; //needs to be initialized
//  //char swap[PAGE_SIZE];
// } memManager;

typedef struct MemEntry MemEntry;

/*
  MemEntry structure for overhead / meta-data for the memory available for allocation/freeing

  MemEntry *prev:.....pointer to previous MemEntry (previous data block's meta-data)
  MemEntry *succ:.....pointer to next MemEntry (next data block's meta-data)
  size_t recognize....value to indicate whether the following data-block is free or not, also a signature denoting end of the struct
                      has two values defined in mymalloc.c to tell if free
  size_t size:........size of data-block following this variable's bytes
*/
struct MemEntry{
  MemEntry * prev, *succ;
  size_t size;
  size_t recognize;
};


static void my_malloc2_init(void ** mem_pool, size_t size, int protection, void * addr);
void my_malloc2_init2(void * mem_pool, size_t size);

void * mymalloc_init();

void * myallocate(unsigned int size, const char* FILENAME, int LINE, int caller);

void mydeallocate(void * ptr, const char* FILENAME, int LINE, int caller);

void *mymalloc2(void * mem_pool, size_t size, const char * file, int line);

void myfree2(void * mem_pool, void *address, const char * file, int line);

#endif