#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

const char * USAGE = "Usage: ./netbench -p [tcp|udp] [-t] -bN\n";
int main(int argc, char** argv){
   int i;
   int prot;
   int threaded;
   int bsize;
   prot = 0;
   threaded = 0;
   bsize = 1;
   for (i=0; i< argc; i++){
      char* ar;
      if ((ar =argv[i])[0] == '-'){
         switch(ar[1]) {
            case 'p':
               i++;
               if (strcmp(argv[i],"tcp") == 0) prot = 1;
               else if (strcmp(argv[i],"udp") == 0) prot = 2;
               else {
                  printf("Invalid Protocol\n%s", USAGE);
                  exit(1);
               }
               break;
            case 't':
               threaded |= 1;
               break;
            case 'b':
               bsize = atoi(ar+2);
               if (!bsize){
                  printf("Invalid size\n%s", USAGE);
                  exit(0);
               }
               break;
            default:
               printf("Invalid Argument\n%s", USAGE);
               exit(0);
         }
      }
   }
   if (!prot){
      printf("%s", USAGE);
      exit(1);
   }
   printf("Protocol: %s\n", prot == 1 ? "TCP" : "UDP");
   printf("Threaded: %s\n", threaded ? "Yes": "No");
   printf("Buffer Size: %d\n", bsize);
   return 0;
}
