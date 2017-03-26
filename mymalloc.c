#include "mymalloc.h"
#include "mymalloc2.h"
#include "my_pthread_t.h"
#include <errno.h> // for debugging errors from mprotect
#include <signal.h>
#include <sys/mman.h>

#define PAGE_SIZE sysconf(_SC_PAGE_SIZE)

//#define SWAP_SIZE 16 * 1024 * 1024
#define ALIGN_PAGE_SIZE(a) (void*)(((size_t)(a)+(pagesize-1))&~(pagesize-1))

static void * system_pool;
//static void * user_pool;

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

void mymalloc_init() {
  printf("Initializing mymalloc\n");
  pagesize = PAGE_SIZE;
  set_mem_handler((void(*)(int, siginfo_t *, void *))ts_mem_handler);
  my_malloc2_init(&system_pool, SYSTEM_POOL_SIZE, PROT_WRITE | PROT_READ | PROT_EXEC, NULL);
  //my_malloc2_init(&user_pool, USER_POOL_SIZE, PROT_NONE);
  //printf("User pool at %p - %p\n", user_pool, (char*)user_pool + USER_POOL_SIZE);
  printf("System pool at %p - %p\n", system_pool, (char*)system_pool + SYSTEM_POOL_SIZE);
}

void * myallocate(unsigned int size, const char* FILENAME, int LINE, int caller) {
  sigset_t current;
  if(caller == LIBRARYREQ || __current_thread->is_main){
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
  if(caller == LIBRARYREQ){
    return myfree2(system_pool, ptr, FILENAME, LINE);
  } else {
    block_signals(&current);
    myfree2(__current_thread->mem_pool, ptr, FILENAME, LINE);

    ublock_signals(&current);
  }
}

