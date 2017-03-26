#ifndef MY_MALLOC2
#define MY_MALLOC2

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

void my_malloc2_init(void ** mem_pool, size_t size, int protection, void * addr);

void my_malloc2_init2(void * mem_pool, size_t size);

void *mymalloc2(void * mem_pool, size_t size, const char * file, int line);

void myfree2(void * mem_pool, void *address, const char * file, int line);

#endif
