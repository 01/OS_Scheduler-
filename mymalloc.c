#if defined(USE_MY_MALLOC)
#include "mymalloc.h"
#include "my_pthread_t.h"

#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)
#define MAX_THREADS 200

#define ALIGN_PAGE_SIZE(a) (void*)(((size_t)(a)+(pagesize-1))&~(pagesize-1))

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_RESET   "\x1b[0m"

static const size_t FREE_SIG = 0xAAAAAAAA;
static const size_t USED_SIG = 0xAAAAAAAB;
#define ALIGN8(a) (((a)+7)&~7)

static void * system_pool;
static void * user_pool; //temporary

static void * threadReservedSpace;
static void * threadPageTables;
static void * globalPageTables;


static size_t pagesize;

static void set_mem_handler(void(*func)(int, siginfo_t *, void *)) {
    struct sigaction act;
    clock_t s, us;

    memset(&act,0,sizeof(act));
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGALRM);
    sigaddset(&act.sa_mask, SIGUSR1);
    act.sa_sigaction = func;
    act.sa_flags = SA_SIGINFO;//|SA_ONSTACK;
    printf("Setting memory handler\n");

    if (sigaction(SIGSEGV, &act, NULL) < 0) {
        perror("sigaction");
        exit(-1);
    }
}

static void ts_mem_handler(int sig, siginfo_t *sip, ucontext_t * current_context) {
  if (!(sip->si_addr >= system_pool && sip->si_addr < (void*)((char*)system_pool + USER_POOL_SIZE))){
    void * page_tail = ALIGN_PAGE_SIZE(sip->si_addr);
    void * page_head = (void*)((char*)page_tail - pagesize);
    mprotect(page_head, pagesize, PROT_READ | PROT_WRITE | PROT_EXEC);
    printf("Enabling use of page at %p\n", page_head);
  }
  else{
    printf("Fatal error. Accessing memory %p\n", sip->si_addr);
    exit(1);
  }
}

// Used to initialize memory for SYSTEM_POOL
static void my_malloc2_init(void ** mem_pool, size_t size, int protection, void * addr){
  char * root = (char *)mmap(addr, size, protection, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  char * ptr = root;
  *mem_pool = root;
  while(ptr < (ptr + (8*1024*1024))){
    * (int *)ptr = (4096 - sizeof(int));
    ptr += PAGE_SIZE;
  }


  // initialize ptrs to key regions in 8MB space
  threadReservedSpace = (char *)(root + PAGE_SIZE * 3) ;
  threadPageTables = (char *)((root) + PAGE_SIZE * THREAD_RESERVED_PAGES_;
  globalPageTables = (char *)(threadPageTables) + PAGE_SIZE * THREAD_PT_PAGES;
  user_pool = (char *)(globalPageTables) + PAGE_SIZE * GLOBAL_PT_PAGES;
}

// Used to initialize memory for USER_POOL
void my_malloc2_init2(void * mem_pool, size_t size){
  char * root = (char *)mem_pool;

  *(int *)root = size - sizeof(int));
}

short getPageForVirtualSlot(short slot) {
  return *(short *)((char *)threadPageTables * 200 * __current_thread->threadID + sizeof(short)*slot);
}

short getCurrentSlotForPage(short pageNum) {
  return *(short *)((char *)globalPageTables + sizeof(short) * pageNum);
}

void * mymalloc_init() {
  printf("Initializing mymalloc\n");
  pagesize = PAGE_SIZE;
  set_mem_handler((void(*)(int, siginfo_t *, void *))ts_mem_handler);
  my_malloc2_init(&system_pool, MEMORY_SIZE, PROT_WRITE | PROT_READ | PROT_EXEC, NULL);
  //my_malloc2_init(&user_pool, USER_POOL_SIZE, PROT_NONE);
  //printf("User pool at %p - %p\n", user_pool, (char*)user_pool + USER_POOL_SIZE);
  printf("System pool at %p - %p\n", system_pool, (char*)system_pool + SYSTEM_POOL_SIZE);
  printf("User pool at %p - %p\n", (char*)system_pool + SYSTEM_POOL_SIZE, (char*)system_pool + MEMORY_SIZE);
  return system_pool;
}

void * myallocate(unsigned int size, const char* FILENAME, int LINE, int caller) {
  sigset_t current;
  if(caller == LIBRARYREQ || __current_thread->is_main){ //main in OS space
  //if(caller == LIBRARYREQ){ //main not in OS space
    return mymalloc2(system_pool, size, FILENAME, LINE);
  } else {
    void * rc;
    block_signals(&current);
    printf("Thread alloc: %s, pool address %p\n", __current_thread->thread_name, __current_thread->mem_pool);
    rc = mymalloc2(__current_thread->mem_pool_front, _current_thread->mem_pool_end, size, FILENAME, LINE);
    ublock_signals(&current);
    return rc;
  }

}

void mydeallocate(void * ptr, const char* FILENAME, int LINE, int caller){
  sigset_t current;
  if(caller == LIBRARYREQ){
    return myfree2(system_pool, ptr, FILENAME, LINE);
  } else {
    block_signals(&current);
    myfree2(__current_thread->mem_pool_front, __current_thread->mem_pool_end,ptr, FILENAME, LINE);

    ublock_signals(&current);
  }
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
void *mymalloc2(void * mempool_front, void * mempool_end,size_t size, const char * file, int line){
  char * root = (char *)mem_pool;
  char * p, *succ;

  // *** algorithm for allocated chunks of memory ***
  p = root;

  do{
    int meta = *(int *)p;
    int blockSize = abs(meta);
    if(abs(blockSize < size) p += blockSize + sizeof(int); // p is not big enough for allocation
    else if(meta < 0) p += blockSize + sizeof(int); // p is not free to be allocated
    else if(blockSize>= size) // found a chunk large enough
    {
      // break off current data-block & make another MemEntry struct
      if(p->size > (size + sizeof(int)){
        * (int *)p = (size * -1)
        if((p+=(sizeof(int) + size) <= memEnd - sizeof(int)) *(int * )p = (memend - p - sizeof(int));
      }
      return (void*)((size_t)p + sizeof(int));
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
void myfree2(void * mem_pool_front, void * mempool_end, void *address, const char * file, int line){
  char *p;
  char * root = (char *)mem_pool;
  p = root;
  int success = 0;
  while(p < (char *)addres){
    if(p == (address - sizeof(int) && (*(int *)p < 0)){
      *(int *)p *= -1;
      success = 1;
    }
  }
    if(!success){
      printf(ANSI_COLOR_RED "free() call failed from %s, line %d\n\tError: Attempted to free nonexistent/already-freed memory.\n" ANSI_COLOR_RESET, file, line);
      return; //address not in memory
    }

//void defrag()
  char * tracker = mempool;
    int meta_size1 = * (int *) tracker;
    int block_size1 = abs.(meta_size1);
    int block_size2 =0;
    int meta_size2 = 0;
    if((tracker + sizeof(int) + block_size1)< mempool_end){
      meta_size2 = * (int *) (tracker + sizeof(int) + block_size1);
      block_size2 = abs(meta_size2);
    }
    else {return;}

    while((tracker + sizeof(int) + block_size1)< mempool_end){      // Conditional loop to cycle through all memory blocks in the main memory
      meta_size1 = * (int *) tracker;
      block_size1 = abs(meta_size1)
      if((tracker + sizeof(int) + block_size1)< mempool_end){
      meta_size2 = * (int *) (tracker + sizeof(int) + block_size1);
      block_size2 = abs(meta_size2)
    else return;

      while(meta_size1 >= 0 && (meta_size2 >=0) && (tracker + sizeof(int) + block_size1)< mempool_end){ // Conditional loop to continue combining neighboring free memory blocks until

        *(int *)tracker += (block_size2 + sizeof(int));
        return;
        meta_size1 = * (int *) tracker;
        block_size1 = abs(meta_size1)
        if((tracker + sizeof(int) + block_size1)< mempool_end){
          meta_size2 = * (int *) (tracker + sizeof(int) + block_size1);
          block_size2 = abs(meta_size2);
        }
        else return;                  // the free memory block does nto have a free neighbor

      }

     tracker += (sizeof(int) + block_size1);
    }
}
}
#endif
