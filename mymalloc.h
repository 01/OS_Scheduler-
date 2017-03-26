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

//#define extern char *error = "Not Enough Space";
#define malloc(x) myallocate(x, __FILE__, __LINE__, THREADREQ);
#define free(x) mydeallocate(x, __FILE__, __LINE__, THREADREQ);
#define USER_POOL_SIZE (1024*1024*7)
#define SYSTEM_POOL_SIZE (1024*1024)
#define ERROR_NOT_ALLOCATED printf("ERROR: Attempted to free an unallocated block. Line: %d File: %s\n", __LINE__, __FILE__);
#define ERROR_NOT_ENOUGH_SPACE printf("ERROR: Attempting to malloc too much space. Line: %d File: %s\n", __LINE__, __FILE__);

typedef enum {LIBRARYREQ, THREADREQ} CallerType;

// typedef struct memManager{
//  void * OS_Region;
//  void * Reserved_Page_Table;
//  void * Heap_Page_Table;
//  void * user_heap; //needs to be initalized
//  void * page_table; //needs to be initialized
//  void * swap_file; //needs to be initialized
//  //char swap[PAGE_SIZE];
// } memManager;

void mymalloc_init();

void * myallocate(unsigned int size, const char* FILENAME, int LINE, int caller);

void mydeallocate(void * ptr, const char* FILENAME, int LINE, int caller);

#endif