#ifndef __MEMORYALLOC_H__
#define __MEMORYALLOC_H__
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>


#define PAGE_SIZE 4096
#define MEMORY_SIZE 8*1024*1024
#define SWAP_SIZE 16*1024*1024
#define ucontext_size sizeof(ucontext_t);


//#define extern char *error = "Not Enough Space";
#define malloc(x) myallocate(x, __FILE__, __LINE__)
#define free(x) mydeallocate(x, __FILE__, __LINE__)
#define ERROR_NOT_ALLOCATED printf("ERROR: Attempted to free an unallocated block. Line: %d File: %s\n", __LINE__, __FILE__);
#define ERROR_NOT_ENOUGH_SPACE printf("ERROR: Attempting to malloc too much space. Line: %d File: %s\n", __LINE__, __FILE__);



void initializeMemory();

void * myallocate(int size, const char* FILENAME, const int LINE);

void mydeallocate(void* ptr, const char* FILENAME, const int LINE);


#endif