#ifndef H_MYPTHREAD
#define H_MYPTHREAD

// Types
typedef struct {
	// Define any fields you might need inside here.
} my_pthread_t;

typedef struct {
	// Define any fields you might need inside here.
} my_pthread_attr_t;

//these are placeholders for your actual implementation
typedef int my_pthread_mutex_t;
typedef int my_pthread_mutexattr_t;


// Functions
int my_pthread_create(my_pthread_t *thread, const my_pthread_attr_t *attr,
			void *(*start_routine) (void *), void *arg);

void my_pthread_exit(void *retval);

int my_pthread_yield(void);

int my_pthread_join(my_pthread_t thread, void **retval);


int my_pthread_mutex_init(my_pthread_mutex_t *mutex,
			const my_pthread_mutexattr_t *attr);

int my_pthread_mutex_destroy(my_pthread_mutex_t *mutex);

int my_pthread_mutex_lock(my_pthread_mutex_t *mutex);

int my_pthread_mutex_unlock(my_pthread_mutex_t *mutex);

#endif
