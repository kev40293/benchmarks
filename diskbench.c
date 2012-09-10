#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#define KB 1024
#define MB 1024*1024
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
double bench_write(struct opts * op) {
	clock_t start, end;
	FILE * file;
	file = fopen("testfile", "w+b");
	char* data = (char*) malloc(sizeof(char)*GB);
	start = clock();
	fwrite(data, (size_t) op->bsize, GB/op->bsize, file);
	end = clock();
	fprintf(stdout, "time: %f\n", diffclock(end,start));
	fclose(file);
	free(data);
	return diffclock(end,start);
}
double bench_read(struct opts * op) {
	clock_t start, end;
	FILE * input;
	input = fopen("/dev/zero", "rb");
	char* data = (char*) malloc(sizeof(char)*GB);
	printf("Reading GB of data\n");
	start = clock();
	fread(data, (size_t) op->bsize, GB/op->bsize, input);
	end = clock();
	fprintf(stdout, "time: %f\n", diffclock(end,start));
	fclose(input);
	free(data);
	return diffclock(end, start);
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
