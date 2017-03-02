#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include "my_pthread_t.h"

#define INTERRUPT_TIME 50*1000 //number of microseconds for itimer to go off

static int isInitialized = 0;
static scheduler * sched = NULL;
static int mutexCount = 0;
static ucontext_t main;

int my_pthread_create(my_pthread_t *thread, my_pthread_attr_t *attr, void *(*function)(void *), void *arg) {
	/* Creates a pthread and sets up its context
	* Mallocs and allocates a stack
	* Makes a Context 
	* Checks if the scheduler is initialized, if its not initialzes the MLQ (make helper)
	* Sets main context initializing pthread
	* if MLQ initialized run algo and add to queue * 1st level i think (might want to make helper)
	*/	
	
	if(!isInitialized){
		initializeScheduler();
		isInitialized = 1;
	}
	if(getcontext(&(thread->thread_context)) < 0){
		printf("Get context fail\n");
		return -1;
	}
	thread->thread_status = RUNNING;
	thread->thread_address = thread;
	thread->threadID = sched->total_threads;
	thread->priority_level = 0;
	initialize_context(&(thread->thread_context));
	makecontext(&(thread->thread_context), (void *)function, 1, arg);
	enqueue(sched->MLQ_Running[thread->priority_level], thread);
	//printf("Thread Status: %s", thread->thread_status);
	sched->total_threads++;
	
	

	//set timer to execute
	struct itimerval it_val;
	timer_set(it_val, 0, INTERRUPT_TIME);

	return 0;
	
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

	//NOTE: If something calls yield that isn't timer_exp, be sure to run kill_timer() here
	
	my_pthread_t * curr = sched->current_thread;
	
    if(curr->thread_status == RUNNING){
    	curr->thread_status = PAUSED;
    }
    if(curr->thread_status == DONE){
    	curr->parent_thread->thread_status = RUNNING;
    	enqueue(sched->MLQ_Running[curr->parent_thread->priority_level], curr->parent_thread);
    	return;
    }
    int new_priority = curr->priority_level + 1;
    if(new_priority > 2) new_priority = 2;
    curr->priority_level = new_priority;
	//loop through queues to find a thread on the 95 queuepr
	int i = 0;
	for(; i < 3; i++){
		if(sched->MLQ_Running[i]->size != 0){
			my_pthread_t *thr = (my_pthread_t *)dequeue(sched->MLQ_Running[i]);
			if(thr->thread_status == ACTIVE){ //should not happen
				printf("Error: Thread %p is running in queue\n", thr);
				exit(1);
			}
			else if(thr->thread_status == BLOCKED){ //mutexed... right?
				enqueue(sched->mutex_list, thr);
			}
			else if(thr->thread_status == DONE){
				enqueue(sched->MLQ_Running[new_priority], curr);
				swapcontext(&(curr->thread_context), &(thr->thread_context));
				my_pthread_exit(thr->return_value);
				break;
			}
			else{
				if(curr->thread_status != DONE) enqueue(sched->MLQ_Running[new_priority], curr);
				swapcontext(&(curr->thread_context), &(thr->thread_context));
				break;
			}
		}
	}	

	// reset timer
	struct itimerval it_val;
	switch(new_priority){
		case 0:
		timer_set(it_val, 0, INTERRUPT_TIME);
		break;
		case 1:
		timer_set(it_val, 0, INTERRUPT_TIME * 2);
		break;
		case 2:
		timer_set(it_val, 0, INTERRUPT_TIME * 4);
		break;
	}
	
	//print address of currently running thread
	
	// return
}

void my_pthread_exit(void *value_ptr) {
	/* Explicit call to the my_pthread_t library tto end thread that called it. If its value ptr isnt NULL
	 * any return from thread will be saved 
	 * 
	 * Send signal to scheduler to let it know its exiting
	 * Change status to done, and yield the main context 
     * 
	 */
	if(sched->current_thread->threadID == 0){
		exit(0);
	}
    sched->current_thread->return_value = value_ptr;
	// value_ptr = sched->current_thread->return_value;
	sched->current_thread->thread_status = DONE;

	my_pthread_yield();
}

int my_pthread_join(my_pthread_t thread, void **value_ptr) {
	// if(thread == NULL){
	// 	printf("Not thread");
	// 	return FAIL;
	// }

	my_pthread_t * join_thread = thread.thread_address;
	join_thread->parent_thread = sched->current_thread;
	sched->current_thread->thread_status = BLOCKED; // same as WAITING
	while(join_thread->thread_status != DONE){
		my_pthread_yield();
	}
	if(value_ptr != NULL){
		*value_ptr = thread.return_value;
		// *value_ptr = sched->current_thread->return_value;
	}
	return 0;
}

int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const my_pthread_mutexattr_t *mutexattr) {
	//printf("Inside my_pthread_mutex_init\n");
    if(mutex == NULL){
    	printf("Invalid Mutex");
        return EINVAL;
    }

    mutex_node * node = (mutex_node *)malloc(sizeof(mutex_node));
	*mutex = node;
    node->mutex_status = UNLOCKED;
    node->waitlist = (queue *)malloc(sizeof(queue));
    node->waitlist->size = 0;
    // No Mutex_List exists, this is first mutex	
    if(mutex_list == NULL){
    	mutex_list = (queue *)malloc(sizeof(queue));
    	mutex_list->size = 0;
		enqueue(mutex_list, node);
    }

    //printf("Mutex Initialized and added to mutex_list\n");

    return 0;
    

}

int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	if(mutex == NULL){
		printf("");
	}
	mutex_node * mutex_to_lock = getMutexNode(*mutex);
	if(mutex_to_lock == NULL){
		printf("Mutex not initialized Cant lock");
		return -1;
	}

	while(__sync_lock_test_and_set(&(mutex_to_lock->mutex_status), LOCKED) == LOCKED){
		// Change calling thread status to WAITING
		
		// Put thread on mutex's waitlist
		printf("Thread is waiting on mutex......\n");
		sched->current_thread->thread_status = BLOCKED;
		enqueue(mutex_to_lock->waitlist, sched->current_thread);

		// Call scheduler
		
	}
	printf("Mutex became unlocked.... free to lock");
	mutex_to_lock->mutex_status = LOCKED;

	return 0;

}

int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
	/* Unlocks a given Mutex
	 * 
	 * Opposite of lock
	 */
	if(mutex == NULL){
		printf("Mutex Pointer is NULL");
		return EINVAL;
	}
	mutex_node * mutex_to_unlock = getMutexNode(*mutex);
	if(mutex_to_unlock == NULL){
		printf("Mutex not initialized\n");
		return EINVAL;
	}
	my_pthread_t * next_thread;
	if(mutex_to_unlock->mutex_status == LOCKED){
		if(mutex_to_unlock->waitlist->size > 0){		
		next_thread = dequeue(mutex_to_unlock->waitlist);
		// add thread to scheduller
		int level = next_thread->priority_level;
		enqueue(sched->MLQ_Running[level], next_thread);
		}
	else{
		mutex_to_unlock->mutex_status = UNLOCKED;
	}
}
	else{
		printf("Mutex is already unlocked\n");
		return -1;
	}
	mutex_to_unlock->mutex_status = UNLOCKED;
	printf("Mutex became unlocked.... free to lock\n");


	return 0;

}

void remove_mutex(queue *q, mutex_node * mutex) {
	mutex_node * prev = NULL;
	mutex_node * ptr = q->head;

	while (ptr) {
		if (ptr == mutex) {
			prev->next = mutex->next;
			free(mutex);
			break;
		}
		prev = ptr;
		ptr = ptr->next;
	}
}

int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
		/* Destorys a given mutex
	 * Mutex must be unlocked before it can be destoryed
	 * Figure just remove from mutex list if its not locked
	 */

	if(mutex == NULL){
		printf("Mutex Pointer is NULL");
		return EINVAL;
	}
	mutex_node * mutex_to_destroy = getMutexNode(*mutex);
	if(mutex_to_destroy== NULL){
		printf("Mutex not initialized\n");
		return -1;
	}

	if(mutex_to_destroy->mutex_status == LOCKED){
		printf("Can not destroy a locked mutex\n");
		return -1;
	}
	remove_mutex(mutex_list, mutex_to_destroy);

	// Decided if reference to node matters or if can do 
	// Trick to switch data with one in front of it 
	// REMOVE MUTEX AND FREE SPACEs

	return 0;


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
		//((mutex_node *)node)->next = NULL;
		q->size++;
		return; 
	}
	else {
		((mutex_node *)(q->tail))->next = node;
	}
	((mutex_node *)node)->next = NULL;
	q->size++;
}

my_pthread_t * dequeue(queue * q) {
	// remove thread from front of queue & return * to thread
	my_pthread_t * front = q->head;
	if(q->size == 1){
		q->size--;
		q->head = q->tail = NULL;
		return front;
	} 
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
	printf("Timer killed.");
	struct itimerval it_val;
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

void initializeScheduler(){
	sched = malloc(sizeof(scheduler));
    sched->MLQ_Running[0] = malloc(sizeof(queue));
    sched->MLQ_Running[1] = malloc(sizeof(queue));
	sched->MLQ_Running[2] = malloc(sizeof(queue));
	sched->mutex_list = malloc(sizeof(queue));
	sched->total_threads = 0;
    init_queue(sched->MLQ_Running[0]);
    init_queue(sched->MLQ_Running[1]);
    init_queue(sched->MLQ_Running[2]);
    init_queue(sched->mutex_list);

	getcontext(&main);
	sched->main_thread = malloc(sizeof(my_pthread_t));
	sched->main_thread->thread_context= main;
	sched->main_thread->thread_context.uc_link = 0;
	sched->main_thread->threadID = 0;
	sched->main_thread->thread_status = ACTIVE;
	sched->total_threads ++;
	sched->mutex_list = mutex_list;
	sched->current_thread = sched->main_thread;
}

mutex_node * getMutexNode(mutex_node * mutex){
	mutex_node * mutex_node_to_return = mutex_list->head;
	while(mutex_node_to_return != NULL) {
		if(mutex_node_to_return == mutex){
			break;
		}
		else{
			mutex_node_to_return = mutex_node_to_return->next; 
		}
	}

	return mutex_node_to_return;
}

void initialize_context(ucontext_t * context){
	//context->uc_link = 0;
	context->uc_stack.ss_sp = malloc(STACK_SIZE);
	context->uc_stack.ss_size = STACK_SIZE;
	context->uc_stack.ss_flags = 0;

}
