#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
const char* usage = "Usage: diskbench -b<block size> [-rt]";
struct opts {
	short read;
	short threaded;
	int bsize;
};
int main(int argc, char** argv){
	printf("Testing disk performance\n");
	int i;
	struct opts * options = (struct opts*) malloc(sizeof(struct opts));
	options->read = 0;
	options->threaded = 0;
	options->bsize = 1;
	for (i =1; i < argc; i++){
		if (argv[i][0] == '-'){
			switch(argv[i][1]){
				case 'b':
					options->bsize = atoi(argv[i]+2);
					if (options->bsize == 0){
						fprintf(stderr, "Invalid Block Size\n");
						exit(1);
					}
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
					fprintf(stderr, "Unrecognized Option\n");
					exit(1);
			}
		}
	}
	printf("Block Size: %dB\n%s Mode\nThreaded: %s\n",
			options->bsize,
			options->read ? "Read" : "Write",
			options->threaded ? "Yes" : "No");
   return 0;
}
