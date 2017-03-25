
#include <ucontext.h>
//#include "my_pthread_t.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define STACK_SIZE 1024*64

// Types
typedef struct {
    ucontext_t current_context;
    char * next_thread;
    int state;
    int thread_id;
    int num_quanta;
    struct timeval first_exec;
    struct timeval last_exec;
    int priority_level;
    void * return_value;
    int page_index;
} my_pthread_t;

typedef struct {

} my_pthread_attr_t;

/* metadata for malloc allocations. Unsigned short allows for 2^16 -1 sizes, by using
 * one bit for free vs allocated allows for 2^15 -1 sizes or 32767 sizes, if another bit is used
 * for other data down line can be 16383 sizes.
 */

typedef unsigned short meta;


typedef struct{
    char * MLQ_Running[3];
    char * wait_queue;
    my_pthread_t main_thread;
    my_pthread_t current_thread;
    int total_threads;
} scheduler;

scheduler * sched;

int main(int argc, char *argv[]) {
   
   printf("Size of uncontext_t: %i\n", sizeof(ucontext_t));
   printf("Size of my_pthread_t %i\n", sizeof(my_pthread_t));
   printf("Size of scheduler %i\n", sizeof(scheduler));
float OS_Thread_Size = (sizeof(ucontext_t) + sizeof(my_pthread_t) + 4096);
  printf("\nTotal Size Ucontext_T + stack + pthread_t struct %i\n", (sizeof(ucontext_t) 
  	+ sizeof(my_pthread_t) + 4096));
  printf("Number of Pages for OS %f\n", (OS_Thread_Size / 4096));
  printf("value of 20 shift 8 bits left plus threadID 20: %i\n", (20<<8) + 20);
  printf("rightshift of value of int created above 8: %i\n", ((20<<8)+20)>>8);
  return 0;
}