#include <ucontext.h>

typedef struct {
    ucontext_t current_context;
    char * next_thread;
    int a;
    int thread_id;
    int num_quanta;
    int first_exec;
    int last_exec;
    int priority_level;
    void * return_value;
    int page_index;
} my_pthread_t;

void testOSAllocated();

int main(int argc, char *argv[]) {
   
  //initializeMemory(); // Passed
 

testOSAllocated();


  return 0;
}

void testOSAllocated(){
	 my_pthread_t test;
int size = sizeof(test);
	my_pthread_t * aptr = myallocate(size, __FILE__, __LINE__);
  aptr = myallocate(size, __FILE__, __LINE__);
  aptr->thread_id = 10;
  char * meta = (char *)aptr -2;

  printf("Allocated thread_id: %i\n", aptr->thread_id);
  printf("MetaData for this thread OS: %i\n", *(int *)meta);
  printf("Address of Thread 1 OS: %i", aptr);
  aptr = myallocate(size, __FILE__, __LINE__);
  * meta = (char *)aptr -2;
  printf("MetaData for this thread OS: %i\n", *(int *)meta);
  printf("Address of Thread 2 OS: %i", aptr);
  aptr = myallocate(size, __FILE__, __LINE__);
  * meta = (char *)aptr -2;
  printf("MetaData for this thread OS: %i\n", *(int *)meta);
  printf("Address of Thread 3 OS: %i", aptr);
  aptr = myallocate(size, __FILE__, __LINE__);
  * meta = (char *)aptr -2;
  printf("MetaData for this thread OS: %i\n", *(int *)meta);
  printf("Address of Thread 4 OS: %i", aptr);
  aptr = myallocate(size, __FILE__, __LINE__);
  * meta = (char *)aptr -2;
  printf("MetaData for this thread OS: %i\n", *(int *)meta);
  printf("Address of Thread 5 OS: %i", aptr);
  aptr = myallocate(size, __FILE__, __LINE__);
  * meta = (char *)aptr -2;
  printf("MetaData for this thread OS: %i\n", *(int *)meta);
  printf("Address of Thread 6 OS: %i", aptr);
  aptr = myallocate(size, __FILE__, __LINE__);
  * meta = (char *)aptr -2;
   printf("MetaData for this thread OS: %i\n", *(int *)meta);
  printf("Address of Thread 7 OS: %i", aptr);
  aptr = myallocate(size, __FILE__, __LINE__);
  * meta = (char *)aptr -2;
  printf("MetaData for this thread OS: %i\n", *(int *)meta);
  printf("Address of Thread 8 OS: %i", aptr);
  aptr = myallocate(size, __FILE__, __LINE__);
  * meta = (char *)aptr -2;
  printf("MetaData for this thread OS: %i\n", *(int *)meta);
  printf("Address of Thread 9 OS: %i", aptr);
  aptr = myallocate(size, __FILE__, __LINE__);
  * meta = (char *)aptr -2;
  printf("MetaData for this thread OS: %i\n", *(int *)meta);
  printf("Address of Thread 10 OS: %i", aptr);
  aptr = myallocate(size, __FILE__, __LINE__);
  * meta = (char *)aptr -2;
  printf("MetaData for this thread OS: %i\n", *(int *)meta);
  printf("Address of Thread 11 OS: %i", aptr);
  aptr = myallocate(size, __FILE__, __LINE__);
  * meta = (char *)aptr -2;
   printf("MetaData for this thread OS: %i\n", *(int *)meta);
  printf("Address of Thread 12 OS: %i", aptr);

}
