#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#define QUANTUM 50000; //number of microseconds for itimer to go off

const int QUANTA_LIMIT[3];
static int isInitialized = 0;
static char memory[1024*1024*8];  // Static memory 8MB
// ucontext_t size on 32 bit computer is 358 bytes, if we allow 1000 threads 358,000 bytes 

static ucontext_t main;

void schedule(){
	//kill previous timer
	struct itimerval itval;
	set_timer(&itval, 0, 0);

	//pick a new thread to run
	my_pthread_t * old = sched->current_thread;
	if(old == NULL){
		if((sched->current_thread = get_thread()) == NULL){
			printf("No threads in queue.\n");
		}
		else{
			sched->current_thread->thread_state = RUNNING;
		}
	}
	else{
		old->num_quanta ++;
		if(old->thread_state == DONE){
			//do nothing?
			if((sched->current_thread = get_thread()) != NULL){
				sched->current_thread->thread_state = RUNNING;
			}
		}
		else if(old->thread_state == YIELDED){
			//stick it to the back of the same queue
			enqueue(sched->MLQ_Running[old->priority_level], old);
			if((sched->current_thread = get_thread()) != NULL){
				sched->current_thread->thread_state = RUNNING;
			}
		}
		else if(old->num_quanta >= QUANTA_LIMIT[old->priority_level]){
			int new_priority = old->priority_level = 2 ? old->priority_level : old->priority_level + 1;
			enqueue(sched->MLQ_Running[new-priority], old);
			if((sched->current_thread = get_thread()) != NULL){
				sched->current_thread->thread_state = RUNNING;
			}
		}
		//else{} we have a thread that has not run its full timeslice, so we don't pick a new thread
	}
	
	//reset timer
	struct itimerval itval;
	set_timer(&itval, 0, QUANTUM);

	//update timestamps
	if(sched->current_thread != NULL){
		gettimeofday(&(sched->current_thread->last_exec));
		if(old != NULL){
			swapcontext(&(sched->current_thread->current_context), &(old->current_context));
			//note if we didn't change a thread, we swapcontext with the same thread
		}
	}
}

//method to get a thread from the first queue if there is one
my_pthread_t * get_thread(){
	int i = 0;
	for(; i < 3; i++){
		if(sched->MLQ_Running[i]->size != 0){
			my_pthread_t * thread = dequeue(sched->MLQ_Running[i]);
			return thread;
		}
	}
	printf("No threads found in queue.\n");
	return NULL;
}

int my_pthread_create(my_pthread_t *thread, my_pthread_attr_t *attr, void *(*function)(void *), void *arg) {
	
}

void my_pthread_yield() { //scheduler will go in here
	sched->current_thread->thread_state = YIELDED;
	schedule();
}

void my_pthread_exit(void *value_ptr) {
	sched->current_thread = DONE;
	sched->current_thread->return_value = value_ptr;
	gettimeofday(&(sched->current_thread->last_exec));
}

int my_pthread_join(pthread_t thread, void **value_ptr) {
	//choose one:

	//1. redefine my_pthread_t to a pointer to another struct (to get around the pass by value)
	//2. locate the thread by its unique(?) thread id

	//once the thread has been found, call my_pthread_yield() until thread(./->)thread_state == DONE)
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

void set_timer(struct itimerval * it_val, int sec, int usec){
	//sets a timer for the chosen amount of time
	it_val.it_interval.tv_sec = 0;
	it_val.it_interval.tv_usec = 0;
	it_val.it_value.tv_sec = sec;
	it_val.it_value.tv_usec = usec;
	signal(SIGALRM, timer_exp);
	setitimer(ITIMER_REAL, it_val, 0);
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
	sched->main_thread->context= main;
	sched->main_thread->context.uc_link = 0;

	sched->main_thread->threadID = 0;
	sched->total_threads ++;
	sched->current_thread = NULL;

}

mutex_node * getMutexNode(int mutex){
	mutex_node * mutex_node_to_return = mutex_list;
	while(mutex_node_to_return ! = NULL) {
		if(mutex_node_to_return->mutex_value == mutex){
			break;
		}
		else{
			mutex_node_to_return = mutex_node_to_return->next; 
		}
	}

	return mutex_node_to_return;
}

void initialize_context(ucontext_t * context){
	context->uc_link = 0;
	context->uc_stack.ss_sp = malloc(STACK_SIZE);
	context->uc_stack.ss_size = STACK_SIZE;
	context->uc_stack.ss_flags = 0;

}

