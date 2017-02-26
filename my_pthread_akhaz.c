#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

int my_pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(*function)(void *), void *arg) {
/* Creates a pthread and sets up its context
 * Mallocs and allocates a stack
 * Makes a Context 
 * Checks if the scheduler is initialized, if its not initialzes the MLQ (make helper)
 * Sets main context initializing pthread
 * if MLQ initialized run algo and add to queue * 1st level i think (might want to make helper)
*/
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

	 int init = 1;
    printf("start initializing mutex\n");
    //mutex = malloc(sizeof(my_pthread_mutex_t));

    if(mutex == NULL){
        return EINVAL;
    }

    mutex->flag = 0;
    //mutex->block = 0;
    mutex->wait = malloc(sizeof(MLQ_Queue)); //make a waiting queue for mutex

;

    printf("mutex was succuesfully initiated\n");

 
}

int my_pthread_mutex_lock(my_pthread_mutex_t *mutex) {
	/* Unlocks a given mutex
	 * 
	 * Keep either seperate list or way to know which mutexs are locked 
	 */

}

int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex) {
	/* Unlocks a given Mutex
	 * 
	 * Opposite of lock
	 */

}

int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex) {
	/* Destorys a given mutex
	 * Mutex must be unlocked before it can be destoryed
	 * Figure just remove from mutex list if its not locked
	 * 
	 */

}