CFLAGS=-Wall -std=c11
SRCS=$(filter-out tmp.c, $(wildcard *.c))
OBJS=$(SRCS:.c=.o)

arcc: $(OBJS)
	$(CC) -o arcc $(OBJS) ./test/foo.o $(LDFLAGS)

$(OBJS): arcc.h

test: arcc test.sh
		./test.sh

clean:
		rm -f arcc *.o *~ tmp* \#* a.out
