CC := gcc
CFLAGS := --pedantic -Wall

all: init unshare setns uname trace

%.o: %.c utils.h font8x8.h
	$(CC) -c $(CFLAGS) -o $@ $<

init: fb.o utils.o
	$(CC) --static fb.o utils.o -o init

unshare: unshare.o utils.o
	$(CC) --static unshare.o utils.o -o unshare

setns: setns.o
	$(CC) --static setns.o -o setns

uname: uname.o
	$(CC) --static uname.o -o uname

trace: trace.o
	$(CC) --static trace.o -o trace

clean:
	rm -f *.o
	rm -f init
	rm -f unshare
	rm -f setns
	rm -f trace
	rm -f uname
