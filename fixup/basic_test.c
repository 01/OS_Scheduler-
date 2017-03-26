#include <stdio.h>
#include "mymalloc.h"

int main(){
	// printf("Allocating...");
	// printf("Got ptr to %p\n", malloc(sizeof(int)));
	char * test = (char *)malloc(sizeof(int));
	printf("Malloc successful, address is %p\n", test);
	printf("Value of %p\n", test);
	*(int*)test = 228;
	printf("Value of %p\n", test);
	
	printf("Value at %p is %d\n", test, *(int*)test);

	void * x = malloc(9999999);
	return 0;
}