CC=gcc
CFLAGS=-g

DEMO=mtsort

default: help

all: basic system

help:
	@echo "We can build in two ways (each of these will build $(DEMO).c):"
	@echo "\tmake basic - Build $(DEMO) with the basic ULT implementation."
	@echo "\tmake system - Build the $(DEMO) using the system pthreads implementation."

basic: $(DEMO).c my_pthread.c my_pthread_t.h
	$(CC) $(CFLAGS) $(DEMO).c my_pthread.c -o $(DEMO)-basic

system: $(DEMO).c
	$(CC) $(CFLAGS) $(DEMO).c -o $(DEMO)-system -pthread -include mypthread-system-override.h

clean:
	@echo "Cleaning..."
	@rm -f $(DEMO)-basic $(DEMO)-system
