CC=gcc
CFLAGS=
LDFLAGS=-lpthread -lm -lrt
BENCHMARKS=membench cpubench diskbench netbench

.SUFFIXES: .o .c

all: $(BENCHMARKS)

.c.o:
	$(CC) ${CFLAGS} -c $<

diskbench: diskbench.o
	$(CC) diskbench.o -o diskbench $(LDFLAGS)

netbench: netbench.o
	$(CC) netbench.o -o netbench $(LDFLAGS)

cpubench: cpubench.o
	$(CC) cpubench.o -o cpubench $(LDFLAGS)

$<: $<.o
	$(CC) $<.o -o $< $(LDFLAGS) 

clean:
	rm -fv *.o $(BENCHMARKS)

