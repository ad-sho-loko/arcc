arcc: arcc.c

test: arcc
	./test.sh

clean:
	rm -f arcc *.o *~ tmp*
