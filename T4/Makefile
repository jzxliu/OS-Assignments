CFLAGS := -g -Wall -Werror -D_GNU_SOURCE -DUSE_PRINTF_IN_SIGNAL_HANDLER
#CFLAGS := -g -Wall -Werror -D_GNU_SOURCE -DUSE_WRITE_IN_SIGNAL_HANDLER
TARGETS := understand_interrupt

# Make sure that 'all' is the first target
all: depend $(TARGETS)

clean:
	rm -rf core *.o $(TARGETS)

realclean: clean
	rm -rf *~ *.bak .depend *.log *.out

tags:
	etags *.c *.h

OBJS := interrupt.o common.o 

$(TARGETS): $(OBJS)

depend:
	$(CC) -MM *.c > .depend

ifeq (.depend,$(wildcard .depend))
include .depend
endif
