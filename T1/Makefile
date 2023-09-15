CFLAGS := -g -Wall -Werror -D_GNU_SOURCE

all: t1-tester selftest

selftest_malloc.o:	selftest_malloc.c
	gcc $(CFLAGS) -c -o selftest_malloc.o selftest_malloc.c

selftest: queue.o selftest_malloc.o selftest.c
	gcc $(CFLAGS) -o selftest queue.o selftest_malloc.o selftest.c

queue.o: queue.c
	gcc $(CFLAGS) -c -o queue.o queue.c

t1-tester: t1-tester.o queue.o malloc.o
	g++ $(CFLAGS) -o t1-tester t1-tester.o queue.o malloc.o

selftest: queue.o selftest_malloc.o selftest.c
	gcc $(CFLAGS) -o selftest queue.o selftest_malloc.o selftest.c

clean:
	rm -rf t1-tester selftest queue.o selftest_malloc.o

