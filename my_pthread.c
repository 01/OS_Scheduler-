#include <ucontext.h>
#include "my_pthread_t.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <time.h>
#include <malloc.h>
#include <sys/mman.h>

#if defined(USE_MY_MALLOC)
#include "mymalloc.h"
#endif

#include <fcntl.h>

#if 0
#if defined(malloc)
#undef malloc
#undef free
#endif
#endif

#if defined(USE_MY_MALLOC)
#undef malloc
#undef free
#define malloc(x) myallocate(x, __FILE__, __LINE__, LIBRARYREQ);
#define free(x) mydeallocate(x, __FILE__, __LINE__, LIBRARYREQ);
#define THREAD_ALLOC -1
#define MUTEX_ALLOC -2
#define MEMSLOT_ALLOC -3
#endif

// From glib
// mapping for general registers in ucontext

struct my_pthread_mutex_int;

typedef struct {
    int ts_quantum;
    int ts_tqexp;
    int ts_slpret;
    int ts_maxwait;
    int ts_lwait;
} disp_t;

typedef struct ts_linked_list {
    my_pthread_int_t * ts_head;
    my_pthread_int_t * ts_tail;
} ts_linked_list_t;

static size_t list_size(const ts_linked_list_t* l) {
    size_t rc = 0;
    const my_pthread_int_t * t = l->ts_head;
    while(t != NULL) {
        t = t->ts_next;
        ++rc;
    }
    return rc; 
}

typedef struct my_pthread_mutex_int {
    my_pthread_int_t * owner;
    ts_linked_list_t wait_queue;
    volatile unsigned char locked;
} my_pthread_mutex_int_t;

#define MAX_PRIORITIES 60
#define DEFAULT_PRIORITY 29
// by assignment we need to create 'high priority' threads
#define CREATE_THREAD_PRIORITY 29
#define UPDATE_DELAY_SECS  5

// threads which are ready to go
static ts_linked_list_t __ready_threads[MAX_PRIORITIES];

// a list of blocked threads 
static ts_linked_list_t __blocked_threads;

// a list of finished threads 
static ts_linked_list_t __finished_threads;

// static data which controls scheduling
static const disp_t __dispatch_control_table[];

#if defined(USE_MY_MALLOC)
static int __file_swap_id;
#endif

// To support starvation worker
static volatile clock_t __current_clock_prev = 0;
static volatile long long __current_clock = 0;
static volatile long long __update_clock = 0;
volatile my_pthread_int_t * __current_thread;

static void extract(ts_linked_list_t * queue, my_pthread_int_t * tsp) {
    if (queue == NULL || tsp == NULL) {
        return;
    }

    if (queue->ts_head == tsp) {
        queue->ts_head = tsp->ts_next;
    }

    if (queue->ts_tail == tsp) {
        queue->ts_tail = tsp->ts_prev;
    }

    if (tsp->ts_next != NULL) {
        tsp->ts_next->ts_prev = tsp->ts_prev;
    }

    if (tsp->ts_prev != NULL) {
        tsp->ts_prev->ts_next = tsp->ts_next;
    }
}


static void insert_at_head(ts_linked_list_t * queue, my_pthread_int_t * tsp) {
    if (tsp == NULL || queue == NULL) {
        return;
    }

    tsp->ts_next = queue->ts_head;

    if (queue->ts_head != NULL) {
        queue->ts_head->ts_prev = tsp;
    }
    queue->ts_head = tsp;

    tsp->ts_prev = NULL;

    if (queue->ts_tail == NULL) {
        queue->ts_tail = tsp;
    }

}

static void insert_at_tail(ts_linked_list_t * queue, my_pthread_int_t * tsp) {
    if (tsp == NULL || queue == NULL) {
        return;
    }

    if (queue->ts_tail != NULL) {
        queue->ts_tail->ts_next = tsp;
    }

    tsp->ts_prev = queue->ts_tail;
    queue->ts_tail = tsp;

    if (queue->ts_head == NULL) {
        queue->ts_head = tsp;
    }

    tsp->ts_next = NULL;

    return;
}

static my_pthread_int_t * extract_top_thread_0(int minimum_priority) {
    my_pthread_int_t * rc;
    size_t i = MAX_PRIORITIES;
    for (; i > 0; --i) {
        if( i < minimum_priority+1 ) {
            break;
        }
        if (__ready_threads[i - 1].ts_head != NULL) {
            ts_linked_list_t * head = __ready_threads + i - 1;
            rc = head->ts_head;
            if( rc ->ts_next == NULL ) {
                head->ts_head = head->ts_tail = NULL;
            } else {
                assert(rc->ts_next != NULL);
                rc->ts_next->ts_prev = NULL;
                head->ts_head = rc->ts_next;
            }
            return rc;
        }
    }
    return NULL;
}

static my_pthread_int_t * extract_top_thread(int minimum_priority) {
    my_pthread_int_t * rc;
    size_t i = MAX_PRIORITIES;
    for (; i > 0; --i) {
        if( i < minimum_priority+1 ) {
            break;
        }
        rc = __ready_threads[i - 1].ts_head;
        for(; rc != NULL;) {
            if( rc->block_until <= __current_clock ) {
                rc->block_until = 0;
                extract(&(__ready_threads[i - 1]),rc);
                return rc;
            } else {
                rc = rc->ts_next;
            }
        }
    }
    return NULL;
}

static my_pthread_int_t * extract_any_thread() {
    my_pthread_int_t * rc;
    size_t i = MAX_PRIORITIES;
    for (; i > 0; --i) {
        rc = __ready_threads[i - 1].ts_head;
        for(; rc != NULL;) {
            extract(&(__ready_threads[i - 1]),rc);
            return rc;
        }
    }
    return NULL;
}

volatile size_t __call;

static void printBlockedThreads() {
    my_pthread_int_t * next_thread;
    if(__blocked_threads.ts_head != NULL ) {
        next_thread = __blocked_threads.ts_head;
        while(next_thread != NULL ) {
            assert(next_thread->ts_blocked==1);
            printf("Blocked %s p:%d\n",next_thread->thread_name,next_thread->ts_priority);
            next_thread = next_thread->ts_next;
        }
    }
}

static void printAllThreads() {
    my_pthread_int_t * t;
    int i;
    printf("Current %s p:%d\n",__current_thread->thread_name,__current_thread->ts_priority);
    printBlockedThreads();
    for(i = MAX_PRIORITIES; i > 0; --i) {
        t = __ready_threads[i-1].ts_head;
        while (t != NULL) {
            assert(t->ts_blocked==0);
            printf("Ready %s p:%d\n",t->thread_name,t->ts_priority);
            t = t->ts_next;
        }
    }

}

//checks starvation once per second
// Quote (assume thread where it is 'process'):
// "called once a second to check the starvation qualities of each job.
//  The routine increments the ts_dispwait counter of every process in the class 
//  (even those that are blocked) by one. If the job is on the ready queue 
//  (i.e., the job is neither running nor blocked), then if its ts_dispwait 
//  exceeds ts_maxwait, the priority and the ts_dispwait value of
//  this process are reset (but not ts_timeleft). Note that this may 
//  involve rearranging the priority queues."

static void ts_update() {
    my_pthread_int_t * t;
    size_t i;
    // no priority change for current thread or blocked ones 
    //printf("ts update() ===========\n");
    //printBlockedThreads();

    ++__current_thread->ts_dispwait;
    t = __blocked_threads.ts_head;
    while(t != NULL ) {
        ++t->ts_dispwait;
        t = t->ts_next;
    }

    // from high to low
    for (i = MAX_PRIORITIES; i > 0; --i) {
        t = __ready_threads[i-1].ts_head;
        while (t != NULL) {
            assert(t != __current_thread);
            //printf("%s p:%d\n",t->thread_name,t->ts_priority);
            if (++t->ts_dispwait >= __dispatch_control_table[i-1].ts_maxwait) {
                t->ts_dispwait = 0;
                int new_priority = __dispatch_control_table[i-1].ts_lwait;
                if (t->ts_priority < new_priority ) {
                    my_pthread_int_t * nxt = t->ts_next;
                    assert(t->ts_priority == i-1);
                    extract(&__ready_threads[i-1], t);
                    insert_at_tail(&__ready_threads[new_priority], t);
                    //printf("%s gets new priority %d(%d)\n",t->thread_name,new_priority,t->ts_priority);
                    t->ts_priority = new_priority;
                    t = nxt;
                    // from high to low order is important
                    // we should not re-calculate this thread again in this call
                    continue;
                }
            }
            t = t->ts_next;
        }
    }
}

#ifndef SYMBOL
#define SYMBOL value
#endif
#if defined(USE_MY_MALLOC)
static void * user_mem = NULL;

#define INIT_SLOT_COUNT 2

static mem_slot_t * __first_slot;
static size_t __allocated_slots;

static void free_slot(mem_slot_t * rc){
    if(rc == NULL){
        return;
    } else {
        rc->next = __first_slot;
        __first_slot = rc;
    }
}

static mem_slot_t * allocate_slot(){
    if(__first_slot == NULL){
        lseek(__file_swap_id, USER_POOL_SIZE*(++__allocated_slots)-1, SEEK_SET);
        write(__file_swap_id, " ", 1);
        LIBRARYREQ = MEMSLOT_ALLOC;
        mem_slot_t * rc = (mem_slot_t*)malloc(sizeof(mem_slot_t));
        rc->offset = (__allocated_slots-1)*USER_POOL_SIZE;
        rc->next = NULL;
        return rc;
    } else {
        mem_slot_t * rc = __first_slot;
        __first_slot = __first_slot->next;
        return rc;
    }
}

static void slot_init(size_t slots){
    size_t i = 0;
    __allocated_slots = slots;
    for(; i < slots; ++i){
        LIBRARYREQ = MEMSLOT_ALLOC;
        mem_slot_t * rc = (mem_slot_t*)malloc(sizeof(mem_slot_t));
        rc->offset = (i)*USER_POOL_SIZE;
        free_slot(rc);
    }
}
#endif

static void ts_init_thread(my_pthread_int_t * t) {
    #if defined(USE_MY_MALLOC)
    static int counter = 0;
    ++counter;
    memset(t, 0, sizeof(my_pthread_int_t));
    #endif
    const disp_t * dt = &(__dispatch_control_table[DEFAULT_PRIORITY]);
    t->ts_timeleft = dt->ts_quantum*10;
    t->start_clock = __current_clock;
    t->ts_priority = DEFAULT_PRIORITY;
    t->ts_dispwait = 0;
    #if defined(USE_MY_MALLOC)
    t->mem_slot = allocate_slot();
    #endif
    //t->mem_pool_location = USER_POOL_SIZE*counter;
    getcontext(&(t->context));
}

static void ts_destroy_thread(my_pthread_int_t * t) {
    if (t->stack_size != 0) {
        munmap(t->stack, t->stack_size);
    }
    free(t);
}

static ucontext_t __when_done_ctx;

static void ts_all_done() {
    exit(0);
}

#define THREAD_STACK_SIZE sysconf(_SC_PAGE_SIZE)
#if !defined(MAP_STACK)
#define MAP_STACK 0
#endif

static void ts_make_stack(my_pthread_int_t * t) {
    t->context.uc_stack.ss_size = THREAD_STACK_SIZE;
    #if defined(USE_MY_MALLOC)
    t->context.uc_stack.ss_sp = myallocate(t->context.uc_stack.ss_size, __FILE__, __LINE__, -4);
    #else
    t->context.uc_stack.ss_sp = 
            (char*)mmap(NULL, t->context.uc_stack.ss_size, PROT_WRITE | PROT_READ | PROT_EXEC, MAP_PRIVATE | MAP_ANONYMOUS| MAP_STACK, -1, 0);
    #endif
    t->stack_size = THREAD_STACK_SIZE;
    if ((char*)MAP_FAILED == t->context.uc_stack.ss_sp) {
        perror("malloc");
        exit(-1);
    }
    t->stack = t->context.uc_stack.ss_sp;
    t->context.uc_stack.ss_size -= 256;
    //printf("Making stack for %s from %p to %p\n",t->thread_name,t->stack, t->stack+t->stack_size);
}


static void set_handler(void(*func)(int, siginfo_t *, void *)) {
    struct itimerval it;
    struct sigaction act;
    clock_t s, us;

    memset(&act,0,sizeof(act));
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGALRM);
    sigaddset(&act.sa_mask, SIGUSR1);
    act.sa_sigaction = func;
    act.sa_flags = SA_SIGINFO;//|SA_ONSTACK;

    if (sigaction(SIGUSR1, &act, NULL) < 0) {
        perror("sigaction");
        exit(-1);
    }
}

static void set_timer(const clock_t ms, void(*func)(int, siginfo_t *, void *)) {
    struct itimerval it;
    struct sigaction act;
    clock_t s, us;

    memset(&act,0,sizeof(act));
    sigemptyset(&act.sa_mask);
    sigaddset(&act.sa_mask, SIGALRM);
    sigaddset(&act.sa_mask, SIGUSR1);
    act.sa_sigaction = func;
    act.sa_flags = SA_SIGINFO;//|SA_ONSTACK;

    if (sigaction(SIGALRM, &act, NULL) < 0) {
        perror("sigaction");
        exit(-1);
    }

    /*
    Convert the milliseconds parameter to seconds and microseconds.
    Set up itimer to go off every tslice_ms time slice.
    */
    s = ms * 1000 / 1000000;
    us = ms * 1000 % 1000000;

    it.it_interval.tv_sec = s;
    it.it_interval.tv_usec = us;
    it.it_value.tv_sec = s;
    it.it_value.tv_usec = us;

    if (setitimer(ITIMER_REAL, &it, NULL) < 0) {
        perror("setitimer(ITIMER_REAL)");
        exit(-1);
    }
}

void block_signals(sigset_t * current) {
    sigset_t mask;
    sigemptyset(&mask);

    sigaddset(&mask, SIGINT);
    sigaddset(&mask, SIGALRM);
    sigaddset(&mask, SIGUSR1);
    if( sigprocmask(SIG_BLOCK, &mask, current) != 0) {
        perror("Cannot block signal");
        exit(-1);
    }
}

void ublock_signals(const sigset_t * current) {
    sigset_t tmp;
    sigemptyset(&tmp);
    sigprocmask(SIG_SETMASK, current, &tmp);
}


static void ts_setup_next_context(ucontext_t * sig_context, my_pthread_int_t * next_thread, my_pthread_int_t * current_thread) {
    // it must be alive thread
    assert(next_thread->is_done == 0);
    // msync(user_mem, USER_POOL_SIZE, MS_SYNC);
    #if 0
    #if defined(USE_MY_MALLOC)
    munmap(user_mem, USER_POOL_SIZE);
    if(next_thread->mem_pool != NULL) {
        // printf("Mapping thread [%s], offset %zu\n", next_thread->thread_name, next_thread->mem_slot->offset);
        void * rc = mmap(user_mem, USER_POOL_SIZE, PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED , __file_swap_id, next_thread->mem_slot->offset);
        assert( rc != MAP_FAILED );
    }    
    #endif
    #endif
    if(current_thread != NULL) {
        swapcontext(&(current_thread->context),&(next_thread->context));
    } else {
       setcontext(&(next_thread->context));
    }

}

/*
  Quote:
"A per process counter, ts_dispwait, is initialized to zero each time a process
is placed back on the dispatcher queue after its time quantum has expired or when it is
awakened (ts_dispwait is not reset to zero when a process is preempted by a higher priority
process). This counter is incremented once per second. If a process�s ts_dispwait value
exceeds the ts_maxwait value for its level, the process�s priority is changed to that indicated
by ts_lwait. The purpose of this field is to prevent starvation."

*/

static volatile char __global_lock = 0;
static volatile size_t __bad_locks  = 0;
static volatile size_t __bad_ulocks  = 0;

static void ts_tick(int sig, siginfo_t *sip, ucontext_t * current_context) {
    ++__call;

    if(__global_lock != 0) {
        return;
    }

    clock_t clk;
    int diff;
    my_pthread_int_t * next_thread;
    my_pthread_int_t * current_thread = (my_pthread_int_t *)__current_thread;

    // assume 64-bit monotonically increasing value on 64-bit platform 
    clk = clock();
    assert(clk >= __current_clock);

    if(sig == SIGALRM && (((clk - __current_clock)*CLOCKS_PER_SEC)/1000) <= 5) {
        // Too fast, was it blocked? 5ms. No need to reschedule that soon.
        return;
    }
    __current_clock = clk;


    //printf("%ld %ld %ld\n",__current_clock,__update_clock,(int64_t)CLOCKS_PER_SEC);
    if(__current_clock >= __update_clock) {
        // future update clock
        __update_clock = __current_clock + UPDATE_DELAY_SECS*CLOCKS_PER_SEC;
        // update priorities to prevent starvation 
        ts_update();
    }

    assert(current_thread != NULL);
    diff = (int)(__current_clock - current_thread->start_clock);
    assert(diff >= 0);

    if(current_thread->is_done) {
        // Current thread signaled that it finished 
        if(current_thread->is_main) {
            // main thread death is a whole process death
            exit((int)(size_t)(current_thread->return_code));
        }
        insert_at_tail(&__finished_threads, current_thread);
        // wake up joining thread
        if( current_thread->join_to != NULL ) { //to
            next_thread = current_thread->join_to; //to
            extract(&__blocked_threads,next_thread);
            if(next_thread->ts_blocked == 0) {
                printf("Thread %s is not blocked\n",next_thread->thread_name);
                assert(0);
            }
            next_thread->ts_blocked = 0;
            // wake up values
            next_thread->ts_priority = __dispatch_control_table[next_thread->ts_priority].ts_slpret;
            next_thread->ts_timeleft = __dispatch_control_table[next_thread->ts_priority].ts_quantum*10;
            next_thread->ts_dispwait = 0;

            insert_at_head(&(__ready_threads[next_thread->ts_priority]), next_thread);

        }

        // just get next thread from ready queue
        next_thread = extract_top_thread(0);
        if( next_thread == NULL ) {
            next_thread = extract_any_thread();
        }
        if( next_thread == NULL ) {
            // Something is wrong. Deadlock?
            printf("All dead. %zd threads in blocked state\n",list_size(&__blocked_threads));
            printBlockedThreads();
            assert(0);
            exit(-1);
        } 
        assert(__current_thread != next_thread);
        __current_thread = next_thread;
        ts_setup_next_context(current_context, next_thread, NULL);

    } else {


        next_thread = NULL;
        if( current_thread->force_yield ) {
            // preempt current thread
            current_thread->force_yield = 0;
            if(current_thread->lock != NULL ) {
                // this lock can be free by this time
                // let's check it 
                my_pthread_mutex_int_t * mutex = current_thread->lock;
                current_thread->lock = NULL;
                if(!mutex->locked ) {
                    mutex->locked = 1;
                    mutex->owner = current_thread;
                    current_thread->lock = NULL;
                    current_thread->join = NULL;
                    return;
                }
                // inversion 
                assert(mutex->owner != NULL);
                assert(mutex->owner != current_thread);
                if( current_thread->ts_priority > mutex->owner->ts_priority) {
                //printf("Inversion!\n");
                    // bump it up
                    if( mutex->owner->ts_blocked ) {
                        // no need to update ready threads
                        mutex->owner->ts_priority = current_thread->ts_priority;
                    } else {
                        extract(&(__ready_threads[mutex->owner->ts_priority]),mutex->owner);
                        mutex->owner->ts_priority = current_thread->ts_priority;
                        insert_at_head(&(__ready_threads[mutex->owner->ts_priority]), mutex->owner);
                    }
                }
                if(mutex->wait_queue.ts_head == NULL ) {
                    mutex->wait_queue.ts_head = mutex->wait_queue.ts_tail = current_thread;
                } else {
                    mutex->wait_queue.ts_tail->ts_mutex_next = current_thread;
                    mutex->wait_queue.ts_tail = current_thread;
                }
                current_thread->ts_mutex_next = NULL;
                current_thread->ts_blocked = 1;

                next_thread = extract_top_thread(0);
                if( next_thread == NULL ) {
                    next_thread = extract_any_thread();
                }
                if( next_thread == NULL ) {
                    printf("Error: dead lock, nothing to run\n");
                    printBlockedThreads();
                    assert(0);
                    exit(-1);
                }
                insert_at_tail(&__blocked_threads,current_thread);

            //printf("(1) Next thread  %s\n",next_thread->thread_name);

            } else {   
                //printf("%ld: -- else 1.1\n",__call);
                if( current_thread->join != NULL ) { //with
                    //printf("%ld: -- else 1.2\n",__call);
                    if( !current_thread->join->is_done ) {
                        //printf("%ld: -- else 1.3\n",__call);
                        current_thread->join->join_to = current_thread;
                        current_thread->ts_blocked = 1;
                        next_thread = extract_top_thread(0);
                        if( next_thread == NULL ) {
                            next_thread = extract_any_thread();
                        }
                        if(next_thread == NULL) {
                            printf("All dead at %d\n", __LINE__);
                            printAllThreads();
                            exit(0);
                        }
                        if(next_thread != NULL) {
                            insert_at_tail(&__blocked_threads,current_thread);
                        }
                    } 
                } else  {
                    next_thread = extract_top_thread(0);
                    if(next_thread != NULL) {
                        // put it in front of it's priority queue or back?
                        // insert_at_head(ts_linked_list_t * queue, my_pthread_int_t * tsp)
                        if( current_thread->ts_priority > DEFAULT_PRIORITY ) {
                            --current_thread->ts_priority;
                        }
                        insert_at_tail(&(__ready_threads[current_thread->ts_priority]), current_thread);
                    }
                }
                current_thread->lock = NULL;
                current_thread->join = NULL;
                if( next_thread == NULL ) {
                    // nothing to do, just let it go
                    current_thread->ts_timeleft = __dispatch_control_table[current_thread->ts_priority].ts_quantum;
                    return;
                }
            }
        } else {

            if( current_thread->ts_timeleft <= diff ) {
                current_thread->ts_priority = __dispatch_control_table[current_thread->ts_priority].ts_tqexp;
                current_thread->ts_timeleft = __dispatch_control_table[current_thread->ts_priority].ts_quantum * 10;
                next_thread = extract_top_thread(current_thread->ts_priority);
                if( next_thread != NULL ) {
                    //printf("(4) Next thread  %s\n",next_thread->thread_name);
                    current_thread->ts_dispwait = 0;     
                    insert_at_tail(&(__ready_threads[current_thread->ts_priority]), current_thread);
                }
            } else {
                current_thread->ts_timeleft -= diff;
                // check if we have high priority in a queue 
                next_thread = extract_top_thread(current_thread->ts_priority+1);
                if( next_thread != NULL ) {
                    //printf("(5) Next thread  %s\n",next_thread->thread_name);
                    // current is preempted but time is not expired yet so let it be in front 
                    insert_at_head(&(__ready_threads[current_thread->ts_priority]), current_thread);
                }
            }
        }

        if( next_thread != NULL ) {
            //printf("%ld: -- else 3\n",__call);

             if(next_thread == current_thread) {
                 printf("Same thread switch %s\n",next_thread->thread_name);
                 assert(0);
             }
             // it must be set during insertion into a queue
             assert( next_thread->ts_timeleft > 0 );
             __current_thread = next_thread;

             //printf("## Switching from %s to %s\n",current_thread->thread_name,next_thread->thread_name);
             ts_setup_next_context(current_context, next_thread, current_thread);
        }
    }
}


void my_pthread_yield(){
    sigset_t current_signal;
    block_signals(&current_signal);
    __current_thread->force_yield = 1;
    __current_thread->join = NULL;
    __current_thread->lock = NULL;
    ublock_signals(&current_signal);
    raise(SIGUSR1);
}

void my_pthread_exit(void *value_ptr){
    sigset_t current_signal;

    //printf("Exit %s\n",__current_thread->thread_name);
    block_signals(&current_signal);

    if(__current_thread->is_main){
        exit((int)(size_t)value_ptr);
    }

    __current_thread->is_done = 1;
    #if defined(USE_MY_MALLOC)
    if(__current_thread->mem_pool != NULL){
        munmap(__current_thread->mem_pool, USER_POOL_SIZE);
    }
    free_slot(__current_thread->mem_slot);
    #endif
    ublock_signals(&current_signal);
    for(;;) {
        // try to send a signal to the dispather
        raise(SIGUSR1);
    }
}

// make this function execute before main
static int ts_init(void) __attribute__((constructor));

static int ts_init() {
    my_pthread_int_t * t;
    printf("Initializing thread\n");
    #if defined(USE_MY_MALLOC)
    __file_swap_id = open("swapfile", O_RDWR | O_CREAT | O_TRUNC, 0777);
    if(__file_swap_id < 0){
        printf("Fatal error: File not created \n");
        exit(1);
    }
    lseek(__file_swap_id, USER_POOL_SIZE*INIT_SLOT_COUNT-1, SEEK_SET);
    write(__file_swap_id, " ", 1);
    void * system_pool = mymalloc_init();
    void * user_pool = (void*)((char*)system_pool + SYSTEM_POOL_SIZE);
    printf("User pool is initialized to %p\n", user_pool);
    slot_init(INIT_SLOT_COUNT);
    LIBRARYREQ = THREAD_ALLOC;
    #endif
    t = (my_pthread_int_t *)malloc(sizeof(my_pthread_int_t));
    // t = (my_pthread_int_t *)myallocate(sizeof(my_pthread_int_t), __FILE__, __LINE__, LIBRARYREQ);
    if (t == NULL) {
        perror("Cannot initialize");
        exit(-1);
    }
    __current_clock = (__current_clock_prev = clock());
    __update_clock = __current_clock + UPDATE_DELAY_SECS*CLOCKS_PER_SEC;
    ts_init_thread(t);
    //t->mem_slot = allocate_slot();
#if defined(USE_MY_MALLOC)
    printf("Thread offset for main is %zu\n", t->mem_slot->offset);
    printf("Mapping to user pool %p\n", user_pool);
    //t->mem_pool = mmap(user_pool, USER_POOL_SIZE, PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED , __file_swap_id, t->mem_slot->offset);
    read(__file_swap_id, user_pool, USER_POOL_SIZE);
    t->mem_pool = user_pool;
    printf("Mapped to user pool %p\n", t->mem_pool);
    my_malloc2_init2(t->mem_pool, USER_POOL_SIZE);
    //mprotect(t->mem_pool, USER_POOL_SIZE, PROT_NONE);
    user_mem = t->mem_pool;
#endif    
    t->is_main = 1;
    __current_thread = t;
    strncpy(t->thread_name,"main",sizeof(t->thread_name));
    #if defined(USE_MY_MALLOC)
    printf("Thread pool init: %s, pool address %p\n", __current_thread->thread_name, __current_thread->mem_pool);
    #endif
    set_handler((void(*)(int, siginfo_t *, void *))ts_tick);
    set_timer(10, (void(*)(int, siginfo_t *, void *))ts_tick);

    return 0;
}

static void thr_func() {
    my_pthread_int_t * t = (my_pthread_int_t *)__current_thread;
    #if defined(USE_MY_MALLOC)
    printf("Thread offset of [%s] is %zu\n", t->thread_name, t->mem_slot->offset);
    t->mem_pool = mmap(user_mem, USER_POOL_SIZE, PROT_WRITE | PROT_READ | PROT_EXEC, MAP_SHARED , __file_swap_id, t->mem_slot->offset);
    my_malloc2_init2(t->mem_pool, USER_POOL_SIZE);
    #endif
    // reset floating point and update fpstate
    void *rc = t->entry_point(t->entry_arg);
    my_pthread_exit(rc);
}

int my_pthread_create(my_pthread_t * thread, my_pthread_attr_t * attr, void *(*function)(void*), void * arg) {
    return my_pthread_create_n(thread,attr,function,arg,"");
}

int my_pthread_create_n(my_pthread_t * thread, my_pthread_attr_t * attr, void *(*function)(void*), void * arg, const char * thread_name) {
    sigset_t current_signal;
    #if defined(USE_MY_MALLOC)
    LIBRARYREQ = THREAD_ALLOC;
    #endif
    *thread = (my_pthread_int_t *)malloc(sizeof(my_pthread_int_t));
    if (*thread == NULL) {
        return -1;
    }

    my_pthread_int_t * t = *thread;
    ts_init_thread(t);
    #if defined(USE_MY_MALLOC)
    strncpy(t->thread_name, thread_name,sizeof(t->thread_name));
    printf("Thread pool init: %s, pool address %p\n", t->thread_name, t->mem_pool);
    #endif
    ts_make_stack(t);
    t->entry_point = function;
    t->entry_arg = arg;
    makecontext(&(t->context), thr_func,0);
//printf("Create %s\n",t->thread_name);
    // override value
    t->ts_priority = CREATE_THREAD_PRIORITY;

    block_signals(&current_signal);

    if( __current_thread->ts_priority < t->ts_priority) {
        insert_at_head(&__ready_threads[t->ts_priority], t);
        ublock_signals(&current_signal);
        raise(SIGUSR1);
        return 0;
    } else {
        // schedule for next run
        insert_at_tail(&__ready_threads[t->ts_priority], t);
        ublock_signals(&current_signal);
        return 0;
    }
}

int my_pthread_join(my_pthread_int_t* thread, void **value_ptr){
    sigset_t current_signal;
    int join_is_done = 0;

    for(;;) {
        block_signals(&current_signal);
        if(thread->is_done) {
            if(value_ptr != NULL){
                *(value_ptr) = thread->return_code;
            }
            extract(&__finished_threads, thread);
            ts_destroy_thread(thread);
            ublock_signals(&current_signal);
            return 0;
        } else {
            if(!join_is_done) {
                join_is_done = 1;
                if(thread->join_to != NULL) { //to
                    // double join
                    ublock_signals(&current_signal);
                    return -1;
                }
                assert( __current_thread != thread );
                __current_thread->join = thread; //with
                //thread->join_to = (my_pthread_int_t* )__current_thread; //to
            }
            __current_thread->force_yield = 1;
            __current_thread->lock = NULL;
            ublock_signals(&current_signal);
            raise(SIGUSR1);
        }
    }
}

int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const my_pthread_mutexattr_t *mutexattr){
    //printf("Initialized mutex.\n");
    if(mutex == NULL){
        assert(0);
        printf("Error: Initialized mutex is null\n");
        return -1;
    }
    #if defined(USE_MY_MALLOC)
    LIBRARYREQ = MUTEX_ALLOC;
    #endif
    *mutex = (my_pthread_mutex_int_t *)malloc(sizeof(my_pthread_mutex_int_t));
    my_pthread_mutex_int_t * mu = *mutex;

    memset(mu, 0, sizeof(my_pthread_mutex_int_t));

    return 0;
}

int my_pthread_mutex_lock(my_pthread_mutex_t *mutex){
    sigset_t current_signal;
    int lock_is_set = 0;
    if(mutex == NULL){
        assert(0);
        printf("Error: Mutex to lock is null\n");
        return -1;
    }
//printf("%s locking %p\n",__current_thread->thread_name,*mutex);

    my_pthread_mutex_int_t * mu = *mutex;
    my_pthread_int_t * t = (my_pthread_int_t *)__current_thread;

    if(__sync_lock_test_and_set(&__global_lock,1) == 0) {
        if(__sync_lock_test_and_set(&mu->locked, 1) == 0) {
            assert(mu->locked == 1);
            assert( mu->owner == NULL);
            assert( mu->wait_queue.ts_head == NULL );
            mu->owner = t;
            __sync_lock_release (&__global_lock);
            return 0;
        }
        __sync_lock_release(&__global_lock);
    }

    for(;;) {
        ++__bad_locks;
        block_signals(&current_signal);
        if( !mu->locked  ) {
            assert( mu->owner == NULL);
            assert( mu->wait_queue.ts_head == NULL );
            mu->locked = 1;
            mu->owner = t;
            ublock_signals(&current_signal);
            return 0;
        } else {
            if( mu->owner == t ) {
                printf("Error: recursive lock\n");
                ublock_signals(&current_signal);
                return -1;
            }
            if(!lock_is_set) {
                lock_is_set = 1;
                __current_thread->lock = mu;
            }
            __current_thread->force_yield = 1;
            __current_thread->join = NULL;
            ublock_signals(&current_signal);
            raise(SIGUSR1);
        }
    }
    return 0;
}

int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex){
    sigset_t current_signal;
    int max_blocked_priority = 0;
    my_pthread_mutex_int_t * mu;
    my_pthread_int_t * t, *tn;
    int counter = 0;


    if(mutex == NULL){
        printf("Error: Mutex to unlock is null\n");
        return -1;
    }

    mu = *mutex;
//printf("%s Un-locking %p\n",__current_thread->thread_name,*mutex);


    if(__sync_lock_test_and_set(&__global_lock,1) == 0) {
        if( mu->wait_queue.ts_head == NULL ) {
            mu->locked = 0 ;
            mu->owner = NULL;
            __sync_lock_release(&__global_lock);
            return;
        }
        __sync_lock_release(&__global_lock);
    }


    // __global_lock = 1;
    // mu->owner = NULL;
    // __sync_lock_release(&mu->locked);
    // if(mu->wait_queue.ts_head == NULL) {
    //     __global_lock = 0;
    //     return;
    // }
    // __global_lock = 0;

    //printf("%s unlocking %p\n",__current_thread->thread_name,*mutex);

    ++__bad_ulocks;

    block_signals(&current_signal);
    assert(mu->locked == 1);
    mu->locked = 0;
    mu->owner = NULL;

    if(mu->wait_queue.ts_head == NULL) {
        // someone did clean already
        ublock_signals(&current_signal);
        return 0;
    }

    // unblock all waiting threads
    t = mu->wait_queue.ts_head;
    for(;t != NULL;) {
        ++counter;
        tn = t->ts_mutex_next;
        if( max_blocked_priority < t->ts_priority ) {
            max_blocked_priority = t->ts_priority;
        }
        extract(&__blocked_threads,t);
        assert(t->ts_blocked==1);
        t->ts_blocked = 0;

    //printf("%s unblocked %s\n",__current_thread->thread_name,t->thread_name);
        insert_at_head(&__ready_threads[t->ts_priority], t); // insert_at_tail does not work well
        t->ts_mutex_next = NULL;
        t = tn;
    } 
    //printf("Unlocked %d threads\n", counter);
    mu->wait_queue.ts_head = mu->wait_queue.ts_tail = NULL;

    //printf("%s unlocked %d threads\n",__current_thread->thread_name,(int)unblocked);

    if( __current_thread->ts_priority < max_blocked_priority ) {
        __current_thread->force_yield = 1;
        __current_thread->join = NULL;
        __current_thread->lock = NULL;
        ublock_signals(&current_signal);
        raise(SIGUSR1);
    } else {
        ublock_signals(&current_signal);
    }
    return 0;
}

int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex){
    //printf("Destroying mutex.\n");
    if(mutex == NULL){
        printf("Error: Mutex to destroy is null\n");
        return -1;
    }

    my_pthread_mutex_int_t * mu = *mutex;

    if(mu->locked){
        printf("Error: Attempt to destroy mutex which is still in use\n");
        exit(-1);
    }
    free(mu);
    return 0;
}

unsigned int sleep(unsigned int seconds) {
    sigset_t current_signal;
    block_signals(&current_signal);
    __current_thread->block_until =  clock() + seconds*CLOCKS_PER_SEC;
    __current_thread->force_yield = 1;
    __current_thread->join = NULL;
    __current_thread->lock = NULL;
    ublock_signals(&current_signal);
    raise(SIGUSR1);
}

///////////////////////////////////////////////////////
//DISPATCH TABLE: index of each 5-element array represents parameters for threads off that priority
//[0] number of quanta to run for
//[1] new priority after done running
//[2] new priority after thread is woken up (from my_pthread_join)
//[3] number of seconds to run for before being aged
//[4] new priority after thread is aged
static const disp_t __dispatch_control_table[MAX_PRIORITIES] = {
    {10, 0, 50, 2, 50}, //0
    {10, 0, 50, 2, 50},
    {10, 0, 50, 2, 50},
    {10, 0, 50, 2, 50},
    {10, 0, 50, 2, 50},
    {10, 0, 50, 2, 50},
    {10, 0, 50, 2, 50},
    {10, 0, 50, 2, 50},
    {10, 0, 50, 2, 50},
    {10, 0, 50, 2, 50},
    {6, 0, 51, 2, 51}, //10
    {6, 1, 51, 2, 51},
    {6, 2, 51, 2, 51},
    {6, 3, 51, 2, 51},
    {6, 4, 51, 2, 51},
    {6, 5, 51, 2, 51},
    {6, 6, 51, 2, 51},
    {6, 7, 51, 2, 51},
    {6, 8, 51, 2, 51},
    {6, 9, 51, 2, 51},
    {5, 10, 52, 2, 52}, //20
    {5, 11, 52, 2, 52},
    {5, 12, 52, 2, 52},
    {5, 13, 52, 2, 52},
    {5, 14, 52, 2, 52},
    {5, 15, 52, 2, 52},
    {5, 16, 52, 2, 52},
    {5, 17, 52, 2, 52},
    {5, 18, 52, 2, 52},
    {5, 19, 52, 2, 52},
    {4, 20, 53, 2, 53}, //30
    {4, 21, 53, 2, 53},
    {4, 22, 53, 2, 53},
    {4, 23, 53, 2, 53},
    {4, 24, 53, 2, 53},
    {4, 25, 54, 2, 54},
    {4, 26, 54, 2, 54},
    {4, 27, 54, 2, 54},
    {4, 28, 54, 2, 54},
    {4, 29, 54, 2, 54},
    {3, 30, 55, 2, 55}, //40
    {3, 31, 55, 2, 55},
    {3, 32, 55, 2, 55},
    {3, 33, 55, 2, 55},
    {3, 34, 55, 2, 55},
    {3, 35, 56, 2, 56},
    {3, 36, 57, 2, 57},
    {3, 37, 58, 2, 58},
    {3, 38, 58, 2, 58},
    {3, 39, 58, 2, 59},
    {3, 40, 58, 2, 59}, //50
    {3, 41, 58, 2, 59},
    {3, 42, 58, 2, 59},
    {3, 43, 58, 2, 59},
    {3, 44, 58, 2, 59},
    {3, 45, 58, 2, 59},
    {3, 46, 58, 2, 59},
    {3, 47, 58, 2, 59},
    {3, 48, 58, 2, 59},
    {2, 49, 59, 32000, 59} //59
};
