#include "mymalloc.h"
#include <errno.h> // for debugging errors from mprotect

static void * MAIN_MEMORY;

#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

typedef struct {
  void * scheduler;
  void * osReserved;
  void * pageTable;
  void * heaps;
} MemManager;

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 

int shiftBits(int value, int shift){
  return (shift > 0) ? (value << shift) : (value >> shift);
}

void init() {
  
  printf("Initializing memory of %ld bytes\n", MEMORY_SIZE);
  MAIN_MEMORY = (void *) memalign(PAGE_SIZE, MEMORY_SIZE);
  if (MAIN_MEMORY == NULL) {
    handle_error("memalign");
  }

  // NOTE: prototype code to check if memory gets protected

  // if (mprotect(MAIN_MEMORY, MEMORY_SIZE, PROT_NONE) == -1) {
  //   handle_error("mprotect");
  // }

  MemManager * memManager = MAIN_MEMORY;
  memManager->osReserved = (MAIN_MEMORY + SCHEDULER_SIZE * PAGE_SIZE);
  memManager->pageTable = (memManager->osReserved + OS_RESERVED_SIZE * PAGE_SIZE);
  memManager->heaps = (memManager->pageTable + GLOBAL_PT_SIZE * PAGE_SIZE);

  // initialize global page table for the heaps
  int i = 0;
  short * ptSlot = (short *) memManager->pageTable;
  for (; i < NUM_PAGES; i++) {
    *ptSlot = i+1;
    ptSlot++;
  }
}

void * myallocate(unsigned int size, const char* FILENAME, const int LINE, int callerType) {
  if (!MAIN_MEMORY) {
    init();
  }

  // TODO: remove this logging message
  printf("Allocating %d bytes - %s - %d - type: %d\n", size, FILENAME, LINE, callerType);

  if (size > MEMORY_SIZE) {
    printf("Request exceeds overall memory space!\n");
    return NULL;
  }

  if ((CallerType)callerType == LIBRARYREQ) {
    // allocate memory for library data
  }
  else if ((CallerType)callerType == THREADREQ) {
    // allocate for thread heaps
  }
}

int main() {
  init();
  // *((int*)MAIN_MEMORY) = 228;
  // printf("found a %d\n", *(int *)MAIN_MEMORY);

  // int * x = malloc(sizeof(int));
  // printf("page size: %d\n", sysconf(_SC_PAGE_SIZE)); // PRINTS 4096 ON LOCAL ENV
  // printf("swap page count: %d\n", SWAP_SIZE / PAGE_SIZE); // PRINTS 4080 pages

  // printf("size of short: %d\n", sizeof(short));
  printf("%d\n", (MEMORY_SIZE - (PAGE_SIZE*402))/PAGE_SIZE);
}