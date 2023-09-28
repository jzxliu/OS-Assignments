.PHONY: clean

CFLAGS := -g -Wall -Werror #-DCOND_AUTO_WAKEUP

all: barrier

%.o : %.c defines.h
	gcc $(CFLAGS) -c $< -o $@

barrier: sync.o errors.o output.o barrier.o 
	gcc $(CFLAGS) -o $@ $^ -lpthread

clean:
	rm -f barrier *.o *~
