#pragma once

#if !defined(my_pthread_create) //for system override

struct my_pthread_int;
typedef  struct my_pthread_int * my_pthread_t;
typedef void *  my_pthread_attr_t;
typedef struct my_pthread_mutex_int * my_pthread_mutex_t;
typedef void * my_pthread_mutexattr_t;


int my_pthread_create( my_pthread_t * thread, my_pthread_attr_t * attr, void *(*function)(void*), void * arg);
void my_pthread_yield();
void my_pthread_exit(void *value_ptr);
int my_pthread_join(my_pthread_t thread, void **value_ptr);
int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const my_pthread_mutexattr_t *mutexattr);
int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);
int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);
int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

#endif
