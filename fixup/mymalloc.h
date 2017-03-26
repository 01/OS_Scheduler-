#ifndef __MEMORYALLOC_H__
#define __MEMORYALLOC_H__
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h> // for memalign
#include <sys/mman.h> // for mprotect
#include <string.h> // for memset
#include <unistd.h>



#include <ctype.h>
#include <time.h>

#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)
#define MEMORY_SIZE 8 * 1024 * 1024
#define SCHEDULER_SIZE 1
#define OS_RESERVED_SIZE 400
#define GLOBAL_PT_SIZE 3
#define HEAP_SLOT_COUNT (MEMORY_SIZE - PAGE_SIZE * (SCHEDULER_SIZE + OS_RESERVED_SIZE + GLOBAL_PT_SIZE))/PAGE_SIZE

#define SWAP_SIZE 16 * 1024 * 1024
#define UCONTEXT_SIZE sizeof(ucontext_t);
// #define NUM_PAGES ((16 + 8) * 1024 * 1024)/PAGE_SIZE
#define NUM_PAGES HEAP_SLOT_COUNT 

typedef enum {LIBRARYREQ, THREADREQ} CallerType;

//#define extern char *error = "Not Enough Space";
#define malloc(x) myallocate(x, __FILE__, __LINE__, THREADREQ);
#define free(x) mydeallocate(x, __FILE__, __LINE__, THREADREQ);
#define ERROR_NOT_ALLOCATED printf("ERROR: Attempted to free an unallocated block. Line: %d File: %s\n", __LINE__, __FILE__);
#define ERROR_NOT_ENOUGH_SPACE printf("ERROR: Attempting to malloc too much space. Line: %d File: %s\n", __LINE__, __FILE__);

// typedef struct memManager{
// 	void * OS_Region;
// 	void * Reserved_Page_Table;
// 	void * Heap_Page_Table;
// 	void * user_heap; //needs to be initalized
// 	void * page_table; //needs to be initialized
// 	void * swap_file; //needs to be initialized
// 	//char swap[PAGE_SIZE];
// } memManager;

void initializeMemory();

void * myallocate(unsigned int size, const char* FILENAME, const int LINE, int caller);

void mydeallocate(void * ptr, const char* FILENAME, const int LINE, int caller);

#endif
