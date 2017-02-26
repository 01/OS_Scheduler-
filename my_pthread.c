#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define INTERRUPT_TIME 50*1000 //number of microseconds for itimer to go off

int my_pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*function)(void *), void *arg) {
	struct itimerval it_val;
	it_val.it_interval.tv_sec = 0;
	it_val.it_interval.tv_usec = 0;
	it_val.it_value.tv_sec = 0;
	it_val.it_value.tv_usec = INTERRUPT_TIME;
	signal(SIGALRM, pthread_yield); //call yield when time is up
	setitimer(ITIMER_REAL, &it_val, 0);


}

void my_pthread_yield() { //scheduler will go in here
	// make scheduling decision
	//if timeslice has run:
		// loop through queues to find a waiting thread
		// swap context
	//else:
		//add 1 to priority
		//queue to corresponding queue

	// reset timer
	signal(SIGALRM, pthread_yield); //call yield when time is up
	struct itimerval it_val;
	it_val.it_interval.tv_sec = 0;
	it_val.it_interval.tv_usec = 0;
	it_val.it_value.tv_sec = 0;
	it_val.it_value.tv_usec = INTERRUPT_TIME;
	//print address of currently running thread
	setitimer(ITIMER_REAL, &it_val, 0);
	// return
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