CC=gcc
CFLAGS=
LDFLAGS=-lpthread
BENCHMARKS=membench cpubench diskbench netbench

.SUFFIXES: .o .c

all: $(BENCHMARKS)

.c.o:
	$(CC) ${CFLAGS} -c $<

diskbench: diskbench.o
	$(CC) diskbench.o -o diskbench $(LDFLAGS)

$<: $<.o
	$(CC) $<.o -o $< $(LDFLAGS) 

clean:
	rm -fv *.o $(BENCHMARKS)

