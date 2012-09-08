#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
const char* usage = "Usage: diskbench [-b[KMG]] [-rt]";
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
							fprintf(stderr,"Invalid Block Size\n%s\n", usage);
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
					fprintf(stderr, "Unrecognized Option\n%s\n", usage);
					exit(1);
			}
		}
	}
	printf("Block Size: %d B\n%s Mode\nThreaded: %s\n",
			options->bsize,
			options->read ? "Read" : "Write",
			options->threaded ? "Yes" : "No");
   return 0;
}
