#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#define GB 1024*1024*1024
const char* usage = "Usage: diskbench [-b[KMG]] [-rt]";
struct opts {
	short read;
	short threaded;
	int bsize;
};
double diffclock(clock_t e, clock_t b){
	return (double) (e-b)*1000/CLOCKS_PER_SEC;
}
int bench_write(struct opts * op) {
	clock_t start, end;
	FILE * file;
	file = fopen("testfile", "w+b");
	char* data = (char*) malloc(sizeof(char)*GB);
	start = clock();
	fwrite(data, (size_t) op->bsize, GB/op->bsize, file);
	end = clock();
	fprintf(stdout, "time: %f\n", diffclock(end,start));
	fclose(file);
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
						case 'g': s = s << 10;
						case 'm': s = s << 10;
						case 'k': s = s << 10;
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
	printf("block size: %d b\n%s mode\nthreaded: %s\n",
			options->bsize,
			options->read ? "read" : "write",
			options->threaded ? "yes" : "no");
	bench_write(options);
   return 0;
}
