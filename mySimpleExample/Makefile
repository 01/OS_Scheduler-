CC=gcc
CFLAGS=-g

basic: $(DEMO).c my_pthread.c my_pthread_t.h
	$(CC) $(CFLAGS) $(DEMO).c my_pthread.c -o $(DEMO)-basic

system: $(DEMO).c
	$(CC) $(CFLAGS) $(DEMO).c -o $(DEMO)-system -pthread -include mypthread-system-override.h

clean:
	@echo "Cleaning..."
	@rm -f $(DEMO)-basic $(DEMO)-system
