#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define INTERRUPT_TIME 50*1000 //number of microseconds for itimer to go off

int my_pthread_create(my_pthread_t *thread, my_pthread_attr_t *attr, void *(*function)(void *), void *arg) {

	// create my_pthread_t struct
	// set status = RUNNING
	// priority = 0
	// return_value = arg

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
	
	/*
	 	check if time sclice for a given thread is completely done
	 	if so:
	 		check status of thread
	 		if done:
				exit thread and save return value into arg
			if not:



	*/

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

void init_queue(queue * q) {
	// queue space already allocated
	q->head = NULL;
	q->tail = NULL;
	q->size = 0;
}

void enqueue(queue * q, my_pthread_t * thread) {
	// insert thread to the end of the queue
	if (q->size == 0) {
		q->head = q->tail = thread;
		thread->next = NULL;
		size++;
	}
	else {
		q->tail->next = thread;
	}
	thread->next = NULL;
	size++;
}

my_pthread_t * dequeue(queue * q) {
	// remove thread from front of queue & return * to thread
	my_pthread_t * front = q->head;
	q->head = front->next;
	front->next = NULL;
	q->size--;
	return front;
}

my_pthread_t * peek(queue * q) {
	// use for checking if queue is empty. if null, queue is empty
	return q->head;		
}

