#if defined(USE_MY_MALLOC)
#include "mymalloc.h"
#include "my_pthread_t.h"

#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)
#define MAX_THREADS 200

// #define OS_RESERVED_PAGES 4
#define THREAD_RESERVED_PAGES 400
#define THREAD_PT_PAGES 200
#define GLOBAL_PT_PAGES 3
#define MUTEX_RESERVED_PAGES 2
// TODO: Include "OS_RESERVED_PAGES + " in the formula ...PAGE_SIZE * (...) if you uncomment OS_RESERVED_PAGES
#define SWAP_SIZE 16 * 1024 * 1024
#define SWAP_SLOT_COUNT SWAP_SIZE/PAGE_SIZE

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
static void * mutexReservedSpace;


static size_t pagesize;
static unsigned char threadCounter = 0;
static unsigned char stackCounter = 0;

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
    //printf("Enabling use of page at %p\n", page_head);
  }
  else{
    printf("Fatal error. Accessing memory %p\n", sip->si_addr);
    exit(1);
  }
}

// Used to initialize memory for SYSTEM_POOL
static void my_malloc2_init(void ** mem_pool, size_t size, int protection, void * addr){
  struct MemEntry * root = (struct MemEntry*)mmap(addr, size, protection, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
  *mem_pool = root;
  root->prev = NULL;
  //root->size = THREAD_RESERVED_PAGES * PAGE_SIZE - sizeof(MemEntry);
  root->size = PAGE_SIZE * MUTEX_RESERVED_PAGES;
  root->recognize = FREE_SIG;

  // initialize ptrs to key regions in 8MB space
  mutexReservedSpace = root;
  threadReservedSpace = (char *)(root) + PAGE_SIZE * GLOBAL_PT_PAGES;
  threadPageTables = (char *)(threadReservedSpace) + PAGE_SIZE * THREAD_RESERVED_PAGES;
  globalPageTables = (char *)(threadPageTables) + PAGE_SIZE * THREAD_PT_PAGES;
  user_pool = (char *)(globalPageTables) + PAGE_SIZE * MUTEX_RESERVED_PAGES;
}

// Used to initialize memory for USER_POOL
void my_malloc2_init2(void * mem_pool, size_t size){
  struct MemEntry * root = (struct MemEntry*)mem_pool;
  root->prev = root->succ = NULL;
  root->size = size - ALIGN8(sizeof(MemEntry));
  root->recognize = FREE_SIG;
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
  if(caller < 0 || __current_thread->is_main){ //main in OS space
  //if(caller < 0){ //main not in OS space
    if(caller == -1){ //thread struct
      if(threadCounter >= 200){
        printf("Out of space. Returning null for newly allocated thread.\n");
        return NULL;
      }
      printf("threadPageTables = %p\n", threadPageTables);
      printf("Allocating thread of size %d in OS space address %p\n", size, (void*)(threadPageTables + (2*PAGE_SIZE*threadCounter)));
      ++threadCounter;
      return (void*)(threadPageTables + (2*PAGE_SIZE*(threadCounter-1)));
    }
    else if(caller == -2){ //mutex struct
      return mymalloc2(system_pool, size, FILENAME, LINE);
    }
    else if(caller == -3){ //pool
      return mymalloc2(system_pool, size, FILENAME, LINE);
    }
    else if(caller == -4){
      if(stackCounter >= 200){
        printf("Out of space. Returning null for newly allocated thread.\n");
        return NULL;
      }
      ++stackCounter;
      return (void*)(threadPageTables + (2*PAGE_SIZE*(stackCounter-1) + 1));
    }
    return mymalloc2(system_pool, size, FILENAME, LINE);
  } else {
    void * rc;
    block_signals(&current);
    printf("Thread alloc: %s, pool address %p\n", __current_thread->thread_name, __current_thread->mem_pool);
    rc = mymalloc2(__current_thread->mem_pool, size, FILENAME, LINE);
    ublock_signals(&current);
    return rc;
  }

}

void mydeallocate(void * ptr, const char* FILENAME, int LINE, int caller){
  sigset_t current;
  if(caller < 0){
    if(ptr >= threadPageTables && ptr <= globalPageTables){ //we never really malloc'd
      return;
    }
    if(__current_thread->is_main){
      return myfree2(system_pool, ptr, FILENAME, LINE);
    }
  } else {
    block_signals(&current);
    myfree2(__current_thread->mem_pool, ptr, FILENAME, LINE);

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
void *mymalloc2(void * mem_pool, size_t size, const char * file, int line){
  struct MemEntry * root = (struct MemEntry*)mem_pool;
  struct MemEntry * p, *succ;

  // *** algorithm for allocated chunks of memory ***
  p = root;
  size = ALIGN8(size);
  printf("Root at %p, size %d\n", root, size);
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

  if(*(size_t*)(address - sizeof(size_t)) == USED_SIG){
    p = (MemEntry*)((size_t)address - ALIGN8(sizeof(MemEntry)));
  }
  else{
    printf(ANSI_COLOR_RED "free() call failed from %s, line %d\n\tError: Attempted to free nonexistent/already-freed memory.\n" ANSI_COLOR_RESET, file, line);
    return; //address not in memory
  }

  p->recognize = FREE_SIG;

  //merge free data blocks into one free data block
  while(p->prev != NULL && p->prev->recognize == FREE_SIG){
    p = p->prev;
  }

  //keep finding prev pointer until no longer free, now merge all free blocks before and after
  while(p->succ != NULL && p->succ->recognize == FREE_SIG){ //merge with additional block
    p->size += p->succ->size + ALIGN8(sizeof(MemEntry));
    p->succ->recognize = 0; //to avoid getting picked up by future addresses
    p->succ = p->succ->succ;
  }
}
#endif