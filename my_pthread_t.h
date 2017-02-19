#ifndef MYPTHREAD_H
#define MYPTHREAD_H

/**
    Creates a pthread that executes function. Attribues are ignored.
*/
int my_pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*function)(void *), void *arg);

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
int my_pthread_join(pthread_t thread, void **value_ptr);

/**
    Initializes a my_pthread_mutex_t created by the calling thread. 
    Attributes are ignored.
*/
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr);

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