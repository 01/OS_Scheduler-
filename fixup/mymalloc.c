#include "mymalloc.h"
#include <errno.h> // for debugging errors from mprotect

// TODO: Remove this errno printout for final version?
// WARNING: When removing this, remove usage throughout this file!!!
#define handle_error(msg) \
           do { perror(msg); exit(EXIT_FAILURE); } while (0)

// ======== GLOBAL STATIC VAR DECLARATIONS ========
static MemManager * memManager;
static void * MAIN_MEMORY;

// TODO: Check if we need this function actually...
static int shiftBits(int value, int shift){
  return (shift > 0) ? (value << shift) : (value >> shift);
}

static void initializeMemory() {
  
  printf("Initializing memory of %ld bytes\n", MEMORY_SIZE);
  MAIN_MEMORY = (void *) memalign(PAGE_SIZE, MEMORY_SIZE);
  if (MAIN_MEMORY == NULL) {
    handle_error("memalign");
  }

  // NOTE: prototype code to check if memory gets protected

  // if (mprotect(MAIN_MEMORY, MEMORY_SIZE, PROT_NONE) == -1) {
  //   handle_error("mprotect");
  // }

  // ============================================================================================
  // |           |                  |               |   Global    |                             |
  // | SCHEDULER |  THREAD STRUCTS  | THREAD Tables | Page Table  |          HEAP SLOTS         |
  // |           |                  |               |             |                             |
  // ============================================================================================

  memManager = MAIN_MEMORY;
  memManager->osReserved = (MAIN_MEMORY + SCHEDULER_SIZE * PAGE_SIZE);
  memManager->threadTables = (memManager->osReserved + THREAD_TABLE_PAGES * PAGE_SIZE);
  memManager->pageTable = (memManager->threadTables + THREAD_RESERVED_SIZE * PAGE_SIZE);
  memManager->heaps = (memManager->pageTable + GLOBAL_PT_SIZE * PAGE_SIZE);

  // initialize all thread tables w/ no ownership (-1)
  int i = 0;
  short *threadSlot = (short *) memManager->threadTables;
  for (; i < 200 * HEAP_SLOT_COUNT; i++) {
    *threadSlot = -1;
    threadSlot++;
  }

  // initialize global page table for the heaps
  i = 0;
  short * ptSlot = (short *) memManager->pageTable;
  for (; i < NUM_PAGES; i++) {
    *ptSlot = i+1;
    ptSlot++;
  }
}

void * myallocate(unsigned int size, const char* FILENAME, const int LINE, int caller) {
  if (!MAIN_MEMORY) {
    initializeMemory();
  }

  // TODO: remove this logging message
  printf("Allocating %d bytes - %s - %d - type: %d\n", size, FILENAME, LINE, caller);

  if (size > MEMORY_SIZE) {
    ERROR_NOT_ENOUGH_SPACE
    return NULL;
  }

  if ((CallerType)caller == LIBRARYREQ) {
    // allocate memory for library data
  }
  else if ((CallerType)caller == THREADREQ) {
    // allocate for thread heaps
  }
  
  // TODO: remove this logging message
  printf("returning addr %p from LIB\n", MAIN_MEMORY);

  // FIXME: Return addr to actual mem allocation
  return MAIN_MEMORY;
}

void mydeallocate(void * ptr, const char* FILENAME, const int LINE, int caller) {
  
}

// Returns a short value corresponding to the slot # where the 
// address belongs in the heap memory space
static short getAddrSlotNum(void *addr) {
  return (addr - memManager->heaps) / PAGE_SIZE;
}