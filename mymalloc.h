#ifndef __MEMORYALLOC_H__
#define __MEMORYALLOC_H__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <unistd.h>


#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)
#define MEMORY_SIZE 8*1024*1024
#define SWAP_SIZE 16*1024*1024
#define ucontext_size sizeof(ucontext_t);

enum {LIBRARYREQ, THREADREQ};

//#define extern char *error = "Not Enough Space";
#define malloc(x) myallocate(x, __FILE__, __LINE__, THREADREQ);
#define free(x) mydeallocate(x, __FILE__, __LINE__, THREADREQ);
#define ERROR_NOT_ALLOCATED printf("ERROR: Attempted to free an unallocated block. Line: %d File: %s\n", __LINE__, __FILE__);
#define ERROR_NOT_ENOUGH_SPACE printf("ERROR: Attempting to malloc too much space. Line: %d File: %s\n", __LINE__, __FILE__);

typedef struct memManager{
	void * OS_Region;
	void * Reserved_Page_Table;
	void * Heap_Page_Table;
	void * user_heap; //needs to be initalized
	void * page_table; //needs to be initialized
	void * swap_file; //needs to be initialized
	//char swap[PAGE_SIZE];
} memManager;

#define numPages ((16 + 8) * 1024 * 1024)/PAGE_SIZE
void initializeMemory();

void * myallocate(unsigned int size, const char* FILENAME, const int LINE, int caller);

void mydeallocate(void * ptr, const char* FILENAME, const int LINE, int caller);


#endif