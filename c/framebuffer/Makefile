CC := gcc
CFLAGS := --pedantic -Wall

all: init unshare setns

%.o: %.c utils.h font8x8.h
	$(CC) -c $(CFLAGS) -o $@ $<

init: fb.o utils.o
	$(CC) --static fb.o utils.o -o init

unshare: unshare.o utils.o
	$(CC) --static unshare.o utils.o -o unshare

setns: setns.o
	$(CC) --static setns.o -o setns

clean:
	rm -f *.o
	rm -f init
	rm -f unshare
	rm -f setns
