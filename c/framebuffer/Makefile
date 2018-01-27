CC := gcc

all: init unshare setns

init: fb.c font8x8.h
	$(CC) --pedantic --static fb.c -o init

unshare: unshare.c
	$(CC) --pedantic --static unshare.c -o unshare

setns: setns.c
	$(CC) --pedantic --static setns.c -o setns

clean:
	rm -f *.o
	rm -f init
	rm -f unshare
	rm -f setns
