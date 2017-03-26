#pragma once

#include <ucontext.h>

#if !defined(my_pthread_create) //for system override

struct my_pthread_int;
typedef  struct my_pthread_int * my_pthread_t;
typedef void *  my_pthread_attr_t;
typedef struct my_pthread_mutex_int * my_pthread_mutex_t;
typedef void * my_pthread_mutexattr_t;

typedef struct my_pthread_int {
    struct my_pthread_int *ts_next;
    struct my_pthread_int *ts_prev;
    struct my_pthread_int *ts_mutex_next;
    struct my_pthread_int *join;    // together with force_yield, transient
    struct my_pthread_mutex_int *lock; // together with force_yield, transient
    struct my_pthread_int *join_to; // after exit what to notify
    long long block_until;
    ucontext_t context;
    void *(*entry_point)(void*);
    void * entry_arg;
    void * return_code;
    void * stack; //consider deleting this one
    void * heap; //for malloc
    size_t stack_size;
    long long start_clock;
    int ts_timeleft; // in ms
    int ts_dispwait;
    char thread_name[64];
    unsigned char ts_priority;
    unsigned char ts_blocked;
    unsigned char is_main;
    unsigned char not_new;
    unsigned char force_yield; // when 1 then 'yield' by signal 
    volatile unsigned char is_done;
} my_pthread_int_t;

// currently active thread (must not be NULL)
static volatile my_pthread_int_t * __current_thread;

int my_pthread_create( my_pthread_t * thread, my_pthread_attr_t * attr, void *(*function)(void*), void * arg);
void my_pthread_yield();
void my_pthread_exit(void *value_ptr);
int my_pthread_join(my_pthread_t thread, void **value_ptr);
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const my_pthread_mutexattr_t *mutexattr);
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

#endif
