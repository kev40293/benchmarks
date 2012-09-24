#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "pthread.h"
#include <sys/time.h>
#include "utils.h"
#define KB 1024
#define MB 1048576
#define GB 1073741824LL
//#define BENCH (unsigned int)1024*1024*512*3
#define BENCH GB
#define NAME "DISKBENCH"
const char* usage = "Usage: diskbench [-b[KMG]] [-rt]";
struct opts {
	short read;
	short threaded;
	long bsize;
};
/*
double getTime_usec() {
   struct timeval tp;
   gettimeofday(&tp, NULL);
   return (double) tp.tv_sec * 1E6 + (double)tp.tv_usec;
}
*/
struct targs {
	FILE * f;
	char * d;
	long bs;
};
#define ARGS ((struct targs*) argt)
void * write_thread(void* argt){
	fwrite (ARGS->d, (size_t) ARGS->bs, BENCH/(ARGS->bs), ARGS->f);
	pthread_exit(NULL);
}

void * read_thread(void * argt) {
   fread(ARGS->d, (size_t) ARGS->bs, BENCH/(ARGS->bs), ARGS->f);
   pthread_exit(NULL);
}

double bench_write(struct opts * op) {
	double start, end;
	FILE * file;
	file = fopen("testfile", "w+b");
	char* data = (char*) malloc(sizeof(char)*BENCH);
	int rc;
	if (op->threaded){
		pthread_t thread;
		struct targs thread_args;
		thread_args.bs = op->bsize;
		thread_args.f = fopen("testfile", "w+b");
		thread_args.d = data;
		void* status;
		printf("Spawning write thread\n");
		start=getTime_usec();
		rc = pthread_create(&thread, NULL, &write_thread, (void*) &thread_args);
		rc += fwrite(data, (size_t) op->bsize, BENCH/op->bsize, file);
		pthread_join(thread, &status);
		end = getTime_usec();
	}
	else {
		start = getTime_usec();
		rc = fwrite(data, (size_t) op->bsize, BENCH/op->bsize, file);
		end = getTime_usec();
	}	
	if (!rc){
		fprintf(stderr, "Write failure %d  :  %lld\n", rc, BENCH);
		exit(1);
	}
	fprintf(stdout, "time: %f\n", (end-start)/1000);
	fclose(file);
	free(data);
	return (end - start) / 1000;
}
double bench_read(struct opts * op) {
	double start, end;
	FILE * input;
	input = fopen("testfile", "w+b");
	char* data = (char*) malloc(sizeof(char)*BENCH);
        fwrite(data, (size_t) op->bsize, BENCH/op->bsize,  input);
        rewind(input);
        //fflush(input);
	printf("Reading GB of data\n");
	int rc;
        if (op->threaded) {
           pthread_t thread;
           struct targs thread_args;
           thread_args.bs = op->bsize;
           thread_args.f = input;
           thread_args.d = data;
           void* status;
           printf("Spawning read thread\n");
           start=getTime_usec();
           rc = pthread_create(&thread, NULL, &read_thread, (void*) &thread_args);
           rc +=fread(data, (size_t) op->bsize, BENCH/op->bsize, input);
           end=getTime_usec();
        }
        else{
           start = getTime_usec();
           rc = fread(data, (size_t) op->bsize, BENCH/op->bsize, input);
           end = getTime_usec();
        }
	if (!rc){
		fprintf(stderr, "Read failure\n");
		exit(1);
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
	long s;
	for (i =1; i < argc; i++){
		if (argv[i][0] == '-'){
			switch(argv[i][1]){
				case 'b':
					s = 1;
					switch (argv[i][2]){
						case 'G': s = s << 10;
						case 'M': s = s << 10;
						case 'K': s = s << 10;
                                                case 'B': s = s;
							break;
						default:
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
                                case 'h':
                                        fprintf(stdout, "%s\n", usage);
                                        exit(0);
				default:
					fprintf(stderr, "unrecognized option\n%s\n", usage);
					exit(1);
			}
		}
	}
	printf("Block size: %lu B\n%s mode\nThreaded: %s\n",
			options->bsize,
			options->read ? "Read" : "Write",
			options->threaded ? "Yes" : "No");
        double time;
        double ratio = BENCH / GB;
	if (options->read){
		time = bench_read(options);
                printf("%s at %f GB/s\n",
                      "Read",
                      1000/time*ratio);
        }
	else{
		time = bench_write(options);
                printf("%s at %f MB/s\n",
                      "Write",
                      1000*1024/time*ratio);
        }
        printf("%s,%ld,%c,%c,%f\n",
           NAME,
		options->bsize,
		options->read ? 'R' : 'W',
		options->threaded ? 'Y' : 'N',
                options->threaded ? (1000/time*BENCH*2) : (1000/time*BENCH));
   return 0;
}
