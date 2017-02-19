#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int my_pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*function)(void *), void *arg) {

}

void my_pthread_yield() {

}

void pthread_exit(void *value_ptr) {

}

int my_pthread_join(pthread_t thread, void **value_ptr) {

}

int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {

}

int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {

}

int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {

}

int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {

}