#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

static isIntialized = 0;

int my_pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*function)(void *), void *arg) {
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

		/* TODO: Insert in Running Queue */

	}

	/*Main Context alrady intiialized */
	my_pthread_t * new_thread = malloc(sizeof(my_pthread_t));
	new_thread->thread_context = malloc(sizeof(ucontext_t));
	getcontext(new_thread->thread_context);
	new_thread->thread_context->uc_link = 0;
	new_thread->thread_context->uc_stack.ss_sp = malloc(STACK_SIZE);
	new_thread->thread_context->uc_stack.ss_size = STACK_SIZE;
	new_thread->thread_context->uc_stack.ss_flags = 0;

	makecontext(new_thread->thread_context, (void *)function, 1, arg);
	
	/* TODO: Add to Running QUEUE */

	return 1;



}

void my_pthread_yield() {
	/* Explicit call to the my_pthread_t scheduler requesting that the current context be 
	 * swapped out and another be scheduled.
	 * 
	 * Check if the current thread, if it is done remove it 
     * Go through queue to find next in line
     * 
     * If active thread is not done, pause it
	 */

}

void pthread_exit(void *value_ptr) 
	/* Explicit call to the my_pthread_t library tto end thread that called it. If its value ptr isnt NULL
	 * any return from thread will be saved 
	 * 
	 * Send signal to scheduler to let it know its exiting
	 * Change status to done, and yield the main context 
     * 
	 */

}

int my_pthread_join(pthread_t thread, void **value_ptr) {
	/* Call to the my_pthread_t library ensuring that the calling thread will not execute until the one it references exits. 
	 * If value_ptr is not null, the return value of the exiting thread will be passed back.
	 *
	 * Not to sure how this works, seems that if thread calls it, it will not 
	 * execute until one its connected to exits, and when it exits if it has a return
	 * value it will be give to this
	 * 
	 * Might have to make struct that is a list of threads that a thread is waiting on
	 */
} 

int my_pthread_mutex_init(my_pthread_mutex_t *mutex, const pthread_mutexattr_t *mutexattr) {
	/* Initializes a mutex created by the calling thread
	 * 
	 * Keep list of mutexs that exist might be best way
	 */
    printf("Inside my_pthread_mutex_init\n");
    if(mutex == NULL){
    	print("Invalid Mutex")
        return EINVAL;
    }

    mutex_node * temp = (mutex_node *)malloc(sizeof(mutex_node));
    temp->mutex_value = *mutex;
    temp->mutex_status = UNLOCKED;
    // No Mutex_List exists, this is first mutex
    if(mutex_list == NULL){		
    	mutex_list = temp;
    }
    else{
    	// Add to Head (order doesnt matter)
    	temp->next = mutex_list

    	mutex_list = temp;
    }
    

;

    printf("Mutex Initialized and added to mutex_list\n");

    return 0;

 
}

int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	/* Unlocks a given mutex
	 * 
	 * Keep either seperate list or way to know which mutexs are locked 
	 */

	mutex_node * mutex_to_lock = getMutexNode(*mutex);
	if(mutex_to_lock == NULL){
		printf("Mutex not initialized");
		return -1;
	}

	while(mutex_to_lock->mutex_status == LOCKED){
		// Endless loop until lock becomes free
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

	mutex_node * mutex_to_unlock = getMutexNode(*mutex);
	if(mutex_to_unlock == NULL){
		printf("Mutex not initialized\n");
		return -1;
	}

	if(mutex_to_unlock->mutex_status == LOCKED){
		mutex_to_unlock->mutex_status = UNLOCKED;
	}
	else{
		printf("Mutex is already unlocked\n");
		return -1;
	}
	printf("Mutex became unlocked.... free to lock");


	return 0;


}

int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	/* Destorys a given mutex
	 * Mutex must be unlocked before it can be destoryed
	 * Figure just remove from mutex list if its not locked
	 */
	mutex_node * mutex_to_destroy = getMutexNode(*mutex);
	if(mutex_to_unlock == NULL){
		printf("Mutex not initialized\n");
		return -1;
	}

	if(mutex_to_unlock->mutex_status == LOCKED){
		printf("Can not destroy a locked mutex\n");
		return -1;
	}

	// Decided if reference to node matters or if can do 
	// Trick to switch data with one in front of it 
	// REMOVE MUTEX AND FREE SPACEs

	return 0;



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