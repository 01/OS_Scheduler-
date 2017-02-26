#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define INTERRUPT_TIME 50*1000 //number of microseconds for itimer to go off

static isIntialized = 0;

int my_pthread_create(my_pthread_t *thread, my_pthread_attr_t *attr, void *(*function)(void *), void *arg) {
	/* Creates a pthread and sets up its context
	* Mallocs and allocates a stack
	* Makes a Context 
	* Checks if the scheduler is initialized, if its not initialzes the MLQ (make helper)
	* Sets main context initializing pthread
	* if MLQ initialized run algo and add to queue * 1st level i think (might want to make helper)
	*/	
	if(!isIntialized){
		isInitialized = 1;
		printf("First thread being created, intialize");
		main_context = malloc(sizeof(ucontext_t));
		getcontext(main_context);

		my_pthread_t * initial_thread = malloc(sizeof(my_pthread_t));
		my_pthread_t->thread_context = main_context;
		initial_thread->thread_status = RUNNING;
		initial_thread->priority_level = 0;

		/* TODO: Insert in Running Queue */
		sched = malloc(sizeof(scheduler));
		sched->MLQ_Running = NULL;
		sched->mutex_queue = NULL;
    	sched->join_queue = NULL;
    	sched->current_thread = initial_thread;

    	sched->MLQ_Running[0] = malloc(sizeof(queue));
    	init_queue(sched->MLQ_Running[0]);
    	init_queue(sched->MLQ_Running[1]);
    	init_queue(sched->MLQ_Running[2]);
	}
	
	/*Main Context alrady intiialized */
	else{
		my_pthread_t * new_thread = malloc(sizeof(my_pthread_t));
		if(getcontext(&(new_thread->context)) < 0){
			printf("Get context fail");
			return FAIL
		}
		initializeContext(new_thread->thread_context);
		makecontext(&(new_thread->thread_context), (void *)function, 1, arg);
		new_thread->thread_status = RUNNING;
		new_thread->priority_level = 0;
	}

	//set timer to execute
	struct itimerval it_val;
	timer_set(it_val, 0, INTERRUPT_TIME);

	return 1;
	
}

void my_pthread_yield() { //scheduler will go in here
	/* Explicit call to the my_pthread_t scheduler requesting that the current context be 
	 * swapped out and another be scheduled.
	 * 
	 * Check if the current thread, if it is done remove it 
     * Go through queue to find next in line
     * 
     * If active thread is not done, pause it
	 */
	//if timeslice has run:
		// loop through queues to find a thread on the running queue
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
	
	my_pthread_t * curr = sched->current_thread;
    if(curr->thread_status == RUNNING){
    	curr->thread_status = PAUSED;
    }
    new_priority = min(curr->priority_level + 1, 2);
	//loop through queues to find a thread on the running queue
	int i = 0;
	for(i = 0; i < 3; i++){
		if(sched->MLQ_Running[i]->size != 0){
			my_pthread_t thr = dequeue(sched->MLQ_Running[i]);
			if(thr->thread_status == RUNNING){ //should not happen
				printf("Error: Thread %p is running in queue\n", thr);
				exit(1);
			}
			else if{thr->thread_status == BLOCKED}{ //mutexed... right?
				enqueue(sched->mutex_queue, thr);
			}
			else if(thr->thread_status = DONE){ //should not happen
				printf("Warning: Thread %p is finished but has been queued\n", thr);
				//do nothing
			}
			else{
				if(curr->thread_status != DONE) enqueue(sched->MLQ_Running[new_priority];
				swapcontext(curr->thread_context, thr->thread_context);
				break;
			}
		}
	}	

	// reset timer
	struct itimerval it_val;
	timer_set(it_val, 0, INTERRUPT_TIME);
	//print address of currently running thread
	
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

void enqueue(queue * q, void * node) {
	// insert thread to the end of the queue
	if (q->size == 0) {
		q->head = q->tail = node;
		node->next = NULL;
		size++;
	}
	else {
		q->tail->next = node;
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

void timer_set(struct itimerval it_val, int sec, int usec){
	//sets a timer for the chosen amount of time
	it_val.it_interval.tv_sec = 0;
	it_val.it_interval.tv_usec = 0;
	it_val.it_value.tv_sec = sec;
	it_val.it_value.tv_usec = usec;
	signal(SIGALRM, timer_exp);
	setitimer(ITIMER_REAL, &it_val, 0);
}

void kill_timer(){
	//aborts the timer by creating a new timer with time 0
	it_val.it_interval.tv_sec = 0;
	it_val.it_interval.tv_usec = 0;
	it_val.it_value.tv_sec = 0;
	it_val.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &it_val, 0);
}

void timer_exp(int signum){
	//time has expired
	my_pthread_yield();

}
