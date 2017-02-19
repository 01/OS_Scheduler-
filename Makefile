FLAGS = -Wall -Werror
DEBUG = -g

all:
    gcc $(FLAGS) mypthread.c -o mypthread

debug:
    gcc $(FLAGS) $(DEBUG) mypthread.c -o mypthread

clean:
    rm mypthread