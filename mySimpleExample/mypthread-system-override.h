/*
 * Author: William Katsak
 */

// This file is used to override mypthread with the "real" system
// pthread implementation.
// Don't touch anything in here.

#ifndef H_MYPTHREAD
#define H_MYPTHREAD
#include <pthread.h>

#define my_pthread_create	pthread_create
#define my_pthread_exit		pthread_exit
#define my_pthread_yield		pthread_yield
#define my_pthread_join		pthread_join

#define my_pthread_mutex_init	pthread_mutex_init
#define my_pthread_mutex_lock	pthread_mutex_lock
#define my_pthread_mutex_trylock pthread_mutex_trylock
#define my_pthread_mutex_unlock	pthread_mutex_unlock
#define my_pthread_mutex_destroy	pthread_mutex_destroy

#define my_pthread_t		pthread_t
#define my_pthread_attr_t	pthread_attr_t

#define my_pthread_mutex_t	pthread_mutex_t
#define my_pthread_mutex_attr_t	pthread_mutex_attr_t

#endif
