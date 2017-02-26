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
#include <uncontext.h>
#include <sys/time.h>

#define STACK 1024*64

enum thread_status {PAUSED, BLOCKED, DONE, RUNNING}


// Types
typedef struct {
    // Define any fields you might need inside here.
    typedef struct my_pthread_t {
    ucontext_t context;
    struct my_pthread_t * next;
    struct my_pthread_mutex_t * mutex_flag; //NULL if no mutex
    enum thread_status status;
    int priority_level;
} my_pthread_t;

// Multilevel Feedback Queue as shown on slide 30 of Lecture 5 Slides
my_pthread_t * scheduler[3];

typedef struct {
    // Define any fields you might need inside here.
    int attr;
} my_pthread_attr_t;

//these are placeholders for your actual implementation
typedef int my_pthread_mutex_t;
typedef int my_pthread_mutexattr_t;


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
void pthread_exit(void *value_ptr);

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
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const m_pthread_mutexattr_t *mutexattr);

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