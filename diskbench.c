#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "pthread.h"
#include <sys/time.h>
#define KB 1024
#define MB 1024*1024
#define GB 1024*1024*1024
#define BENCH GB
const char* usage = "Usage: diskbench [-b[KMG]] [-rt]";
struct opts {
	short read;
	short threaded;
	int bsize;
};
double getTime_usec() {
   struct timeval tp;
   gettimeofday(&tp, NULL);
   return (double) tp.tv_sec * 1E6 + (double)tp.tv_usec;
}
struct targs {
	FILE * f;
	char * d;
	int bs;
};
#define ARGS ((struct targs*) argt)
void * write_thread(void* argt){
	fwrite (ARGS->d, (size_t) ARGS->bs, BENCH/2/(ARGS->bs), ARGS->f);
	pthread_exit(NULL);
}

void * read_thread(void * argt) {
   fread(ARGS->d, (size_t) ARGS->bs, BENCH/2/(ARGS->bs), ARGS->f);
   pthread_exit(NULL);
}

double bench_write(struct opts * op) {
	double start, end;
	FILE * file;
	file = fopen("testfile", "w+b");
	char* data = (char*) malloc(sizeof(char)*BENCH);
	if (op->threaded){
		pthread_t thread;
		struct targs thread_args;
		thread_args.bs = op->bsize;
		thread_args.f = file;
		thread_args.d = data + BENCH/2;
		void* status;
		printf("Spawning write thread\n");
		start=getTime_usec();
		pthread_create(&thread, NULL, &write_thread, (void*) &thread_args);
		fwrite(data, (size_t) op->bsize, BENCH/2/op->bsize, file);
		pthread_join(thread, &status);
		end = getTime_usec();
	}
	else {
		start = getTime_usec();
		fwrite(data, (size_t) op->bsize, BENCH/op->bsize, file);
		end = getTime_usec();
	}	
	fprintf(stdout, "time: %f\n", 1000*(end-start));
	fclose(file);
	free(data);
	return (end - start) * 1000;
}
double bench_read(struct opts * op) {
	double start, end;
	FILE * input;
	input = fopen("/dev/zero", "rb");
	char* data = (char*) malloc(sizeof(char)*BENCH);
	printf("Reading GB of data\n");
        if (op->threaded) {
           pthread_t thread;
           struct targs thread_args;
           thread_args.bs = op->bsize;
           thread_args.f = input;
           thread_args.d = data + BENCH/2;
           void* status;
           printf("Spawning read thread\n");
           start=getTime_usec();
           pthread_create(&thread, NULL, &read_thread, (void*) &thread_args);
           fread(data, (size_t) op->bsize, BENCH/2/op->bsize, input);
           end=getTime_usec();
        }
        else{
           start = getTime_usec();
           fread(data, (size_t) op->bsize, BENCH/op->bsize, input);
           end = getTime_usec();
        }
	fprintf(stdout, "time: %f\n", (end-start)/1000);
	fclose(input);
	free(data);
	return (end - start)/1000;
}
	
int main(int argc, char** argv){
	printf("testing disk performance\n");
	int i;
	struct opts * options = (struct opts*) malloc(sizeof(struct opts));
	options->read = 0;
	options->threaded = 0;
	options->bsize = 1;
	int s;
	for (i =1; i < argc; i++){
		if (argv[i][0] == '-'){
			switch(argv[i][1]){
				case 'b':
					s = 1;
					switch (argv[i][2]){
						case 'G': s = s << 10;
						case 'M': s = s << 10;
						case 'K': s = s << 10;
							break;
						default: s = s;
							fprintf(stderr,"invalid block size\n%s\n", usage);
							exit(1);
					}
					options->bsize = s;
					break;
				case 'r':
					options->read |= 1;
					if (argv[i][2] == 't')
						options->threaded |= 1;
					break;
				case 't':
					options->threaded |= 1;
					if(argv[i][2] == 'r')
						options->read |= 1;
					break;
				default:
					fprintf(stderr, "unrecognized option\n%s\n", usage);
					exit(1);
			}
		}
	}
	printf("Block size: %d B\n%s mode\nThreaded: %s\n",
			options->bsize,
			options->read ? "Read" : "Write",
			options->threaded ? "Yes" : "No");
	if (options->read)
		bench_read(options);
	else
		bench_write(options);
   return 0;
}
