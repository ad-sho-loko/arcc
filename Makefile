CFLAGS=-Wall -std=c11
SRCS=$(wildcard *.c)
OBJS=$(SRCS:.c=.o)

arcc: $(OBJS)
	$(CC) -o arcc $(OBJS) $(LDFLAGS)

$(OBJS): arcc.h

test: arcc
		./test.sh

clean:
		rm -f arcc *.o *~ tmp* \#*
