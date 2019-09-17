ifdef CROSS_COMPILE
	CC=$(CROSS_COMPILE)gcc
endif

CFLAGS=-Os -Wall -DDEBUG
PROGS=timestamp

all: $(PROGS)

clean:
	rm -f $(PROGS) *.o
