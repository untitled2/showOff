C=gcc
CFLAGS=-std=c99 -Wall -Wextra -pedantic
OBJS=list.o apriori.o
PROGRAM=apriori

all: $(PROGRAM)

$(PROGRAM):	list.o apriori.o
						$(CC) -o $@ $^

list.o:	list.c
apriori.o:	apriori.c

clean:
	rm -f $(OBJS) $(PROGRAM)
