CC = gcc
CFLAGS = -Wall -g
TARGETS = dine 

all: $(TARGETS)

dine: dine.c dawdle.o
	$(CC) $(CFLAGS) -o $@ $^ -lpthread

dawdle.o: dawdle.c
	$(CC) $(CFLAGS) -c $^

clean:
	rm $(TARGETS)
	rm -dr dine.dSYM
