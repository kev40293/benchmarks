CC=gcc
BENCHMARKS=membench cpubench diskbench netbench

.SUFFIXES: .o .c

all: $(BENCHMARKS)

.c.o:
	$(CC) ${CFLAGS} -c $<

$<: $<.o
	$(CC) $<.o -o $<

clean:
	rm -fv *.o $(BENCHMARKS)

