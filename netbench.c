#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#define PORT 10000
#define ADDRESS "127.0.0.1"
#define TEST_SIZE 1024LL*1024LL*1024LL

const char * USAGE = "Usage: ./netbench -p [tcp|udp] [-t] -bN\n";

double getTime_usec() {
   struct timeval tp;
   gettimeofday(&tp, NULL);
   return (double) tp.tv_sec * 1E6 + (double)tp.tv_usec;
}

void run_server(int bsize, int protocol){
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
   printf("Listening on localhost at port 10000\n");
   listen(sock, 5);
   while (protocol == SOCK_DGRAM){
      struct sockaddr_in udpsock;
      char * buf;
      buf = (char*) malloc(sizeof(char)*bsize);
      int size = sizeof(sock);
      while (recvfrom(sock, &buf, bsize, 0, (struct sockaddr *)&udpsock, &size) > 0);
      free(buf);
   }
   while (protocol == SOCK_STREAM){
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
      int rc;
      //int start, end;
      //start = getTime_usec();
      while((rc = recv(newsock, buffer, bsize, 0)) > 0) { }
      //end = getTime_usec();
      if (rc < 0){
         perror("Failed read\n");
      }
      /*
      double netspeed = TEST_SIZE/2E9/(end-start)*8*10E6;
      netspeed /= (end-start);
      char unit = 'G';
      if (netspeed < 1){
         unit = 'M';
         netspeed *= 2E10;
      }
      //printf("Read %lld in %f milliseconds\n", TEST_SIZE, (end-start)*10E3);
      //printf("Downspeed: %f %cbits/second\n", netspeed, unit);
      */
      free(buffer);
      close(newsock);
      exit(0);
   }
   close(sock);
}

void run_client(int bsize, int protocol){
   int sock;
   struct sockaddr_in serv;
   sock = socket(AF_INET, protocol, 0);
   serv.sin_family = AF_INET;
   serv.sin_port = htons(PORT);
   inet_pton(AF_INET, ADDRESS, &(serv.sin_addr));
   if (connect(sock, (struct sockaddr *) &serv, sizeof(serv))< 0){
      perror("Connect Failed\n");
      exit(1);
   }
   char *buffer;
   buffer = (char*) malloc(sizeof(char)*bsize);
   int i;
   i = TEST_SIZE/bsize;
   printf("Writing data over network\n");
   double start,end;
   start = getTime_usec();
   for (; i > 0; i--){
        if (write(sock,buffer, bsize) < 0){
           perror("Failed write\n");
           exit(1);
        }
   }
   end = getTime_usec();
   printf("Write %lld in %f milliseconds\n", TEST_SIZE, (end-start)/1E3);
   printf("Upspeed: %f Gbits/second\n", TEST_SIZE/2E9*8*1E6/(end-start));
   close(sock);
   free(buffer);
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
   int p;
   if ((p = fork()) == 0){
        run_server(bsize, prot);
   }
   sleep(2);
   run_client(bsize, prot);
   if (prot == SOCK_STREAM)
        wait(p);
   else
      kill(p, 9);
   return 0;
}
