#ifndef MYPTHREAD_H
#define MYPTHREAD_H

// Author: John-Austen Francisco
// Date: Feb 2017
//
// Team: 
// Arpit Shah (aps180)
// Andrew Khaz (akhaz)
// Mikhail Soumar (ms2237)
// 
// Ilab machine used: vi.cs.rutgers.edu

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <signal.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sched.h>
#include <ucontext.h>
#include <sys/time.h>

#define STACK_SIZE 1024*64

/*************************/

enum thread_status {PAUSED, BLOCKED, DONE, RUNNING, ACTIVE};
enum mutex_status {LOCKED, UNLOCKED};

//these are placeholders for your actual implementation
typedef int my_pthread_mutex_t;
typedef int my_pthread_mutexattr_t;

// Types
typedef struct my_pthread_t {
    struct my_pthread_t * next;
    int threadID;
    ucontext_t  thread_context;
    struct my_pthread_t * thread_address;
    struct my_pthread_t * parent_thread; 
    // struct my_pthread_mutex_t * mutex_flag; // NULL if no mutex
    enum thread_status thread_status;
    int priority_level;
    void * return_value;
} my_pthread_t;

typedef struct queue {
    void * head;
    void * tail;
    int size;
} queue;


typedef struct {

} my_pthread_attr_t;

typedef struct {
    queue * MLQ_Running[3];
    queue * mutex_list;
    my_pthread_t * current_thread;
    int total_threads;
    my_pthread_t * main_thread;
} scheduler;



typedef struct my_pthread_mutex_t_node{
    struct mutex_node * next;           // Pointer to next mutex in mutex list      
    my_pthread_mutex_t  mutex_value;     // Mutex value
    enum mutex_status mutex_status;                   // Status locked or unlocked
    struct queue * waitlist;      // List of threads waiting on this mutex

} mutex_node;

struct queue * mutex_list;

mutex_node * getMutexNode(int mutex);
void initializeScheduler();
void initialize_context(ucontext_t * context);

void init_queue(queue * q);
void enqueue(queue * q, void * thread);
my_pthread_t * dequeue(queue * q);
my_pthread_t * peek(queue * q); // use for checking if queue is empty. if null, queue is empty

void timer_set(struct itimerval it_val, int sec, int usec);
void kill_timer();
void timer_exp(int signum);
/*
/**
    Creates a pthread that executes function. Attribues are ignored.
*/
int my_pthread_create(my_pthread_t *thread, my_pthread_attr_t *attr, void *(*function)(void *), void *arg);

/**
    Explicit call to the my_pthread_t scheduler requesting that the 
    current context be swapped out and another be scheduled.
*/
void my_pthread_yield();

/**
    Explicit call to the my_pthread_t library to end the pthread that 
    called it. If the value_ptr isn't NULL, any return value from the 
    thread will be saved.
*/
void my_pthread_exit(void *value_ptr);

/**
    Call to the my_pthread_t library ensuring that the calling thread 
    will not execute until the one it references exits. If value_ptr 
    is not null, the return value of the exiting thread will be passed back.
*/
int my_pthread_join(my_pthread_t thread, void **value_ptr);

/**
    Initializes a my_pthread_mutex_t created by the calling thread. 
    Attributes are ignored.
*/
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const my_pthread_mutexattr_t *mutexattr);

/**
    Locks a given mutex, other threads attempting to access this 
    mutex will not run until it is unlocked.
*/
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);

/**
    Unlocks a given mutex.
*/
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);

/**
    Destroys a given mutex. Mutex should be unlocked before doing so.
*/
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

#endif