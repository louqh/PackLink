CC=gcc
CFLAGS=-Wall -Wextra -g

all: rbtree

rbtree: rbtree.o main.o
	$(CC) $(LDFLAGS) rbtree.o main.o -o rbtree

rbtree.o: rbtree.c
	$(CC) $(CFLAGS) -c rbtree.c -o rbtree.o

main.o: main.c
	$(CC) $(CFLAGS) -c main.c -o main.o

clean:
	rm -f *.o
	rm -f *~
	rm -f rbtree