#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define PORT 10000
#define ADDRESS "127.0.0.1"

const char * USAGE = "Usage: ./netbench -p [tcp|udp] [-t] -bN\n";

void server(int bsize, int protocol){
   int sock;
   struct sockaddr_in serv;
   sock = socket(AF_INET, protocol, 0);
   if (sock < 0){
      perror("Server creation failed");
      exit(1);
   }
   serv.sin_family = AF_INET;
   serv.sin_port = htons(PORT);
   serv.sin_addr.s_addr = INADDR_ANY;
   if (bind(sock, (struct sockaddr *) &serv, sizeof(serv)) < 0){
      perror("Bind Fail/n");
      exit(1);
   }
   listen(sock, 5);
   while (1){
      int addrlen, newsock;
      struct sockaddr_in inadd;
      addrlen = sizeof(struct sockaddr_in);
      newsock = accept(sock, (struct sockaddr *)&inadd, &addrlen);
      if (newsock < 0){
         perror("failed accept");
         exit(1);
      }
      char *buffer;
      buffer = (char*)malloc(sizeof(char)*bsize);
      while(recv(newsock, buffer, bsize, 0) == bsize) {}
      printf("%s\n", buffer);
   }
}

void client(int bsize, int protocol){
}

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
               if (strcmp(argv[i],"tcp") == 0) prot = SOCK_STREAM;
               else if (strcmp(argv[i],"udp") == 0) prot = SOCK_DGRAM;
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
   printf("Protocol: %s\n", prot == SOCK_STREAM ? "TCP" : "UDP");
   printf("Threaded: %s\n", threaded ? "Yes": "No");
   printf("Buffer Size: %d\n", bsize);
   server(bsize, prot);
   return 0;
}
