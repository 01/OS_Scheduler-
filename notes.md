# Initial Project Notes

We need to create a scheduler which switches between different contexts at various time intervals.

Towards that goal, we need to maintain the various threads and keep track of their priorities.

To switch at every interval, the assignment suggests the usage of a signal handler to interrupt normal execution every 50ms quanta since the signal firing itself will pause the current context.

We have the following tasks which we need to complete:
* Design a multilevel priority queue for the threads
    * determine the properties & actual implementation
    * highest priority to new threads
    * take time duration into account
        * **as threads decrease in priority, they run less often but longer**
        * need to determine how long a thread has been running
* Design the mutex struct for later usage in the library implementation
    * my_pthread_mutex_init
    * my_pthread_mutex_lock
    * my_pthread_mutex_unlock
    * my_pthread_mutex_destroy
* Implement the various functions necessary in the library implementation
    * my_pthread_create
    * my_pthread_yield
    * pthread_exit
    * my_pthread_join

Each time the signal handler is executed, we need to check for the current thread's execution time, whether it has completed one timeslice and act accordingly whether that may be context swap and reassigning priorities or simply wait for another execution interruption.