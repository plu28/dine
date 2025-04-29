CC = gcc
CFLAGS = -Wall -g
TARGETS = dine

all: $(TARGETS)

dine: dine.c
	$(CC) $(CFLAGS) -o $@ $^

clean:
	rm $(TARGETS)
