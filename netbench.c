#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <math.h>
#define PORT 10000
#define ADDRESS "127.0.0.1"
//#define TEST_SIZE 1024LL*1024LL*1024LL
#define DURATION 10

const char * USAGE = "Usage: ./netbench -p [tcp|udp] [-t] -bN [-s] [-c <ipaddress>]\n";

double getTime_usec() {
   struct timeval tp;
   gettimeofday(&tp, NULL);
   return (double) tp.tv_sec * 1E6 + (double)tp.tv_usec;
}
unsigned long string_to_bytes(char* bstring){
   int last = strlen(bstring) - 1;
   char end;
   int scalar = 1;
   end = bstring[last];
   printf("data = %s %c\n", bstring, end);
   switch (end){
      case 'B':
         bstring[last] = '\0';
         scalar = atoi(bstring);
         break;
      case 'K':
         bstring[last] = '\0';
         scalar = atoi(bstring) << 10;
         break;
      case 'M':
         bstring[last] = '\0';
         scalar = atoi(bstring) << 20;
         break;
      case 'G':
         bstring[last] = '\0';
         scalar = atoi(bstring) << 30;
         break;
      default:
         scalar = atoi(bstring);
   }
   return scalar;
}

char* calc_rate(int buffersize, int itr, double sec){
   double rate = 8.0*itr*buffersize/sec;
   int pow = 0;
   while (rate > 1024){
      rate /= 1024.0;
      pow++;
   }
   char *buf = (char*) malloc(sizeof(char)*30);
   char blue;
   switch (pow){
      case 0:
         blue = ' ';
         break;
      case 1:
         blue = 'K';
         break;
      case 2:
         blue = 'M';
         break;
      case 3:
         blue = 'G';
         break;
   }
   sprintf(buf, "%f %c", rate, blue);
   return buf;
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
   while (protocol == SOCK_RAW){
      struct sockaddr_in pingsock;
      char buf[bsize];
      int size = sizeof(sock);
      while (recvfrom(sock, &buf, bsize, 0, (struct sockaddr *)&pingsock, &size) > 0){
         sendto(sock, &buf, bsize, 0, (struct sockaddr *) &pingsock, size);
      }
      close(sock);
      exit(0);
   }
   listen(sock, 5);
   printf("Listening on localhost at port 10000\n");

   while (protocol == SOCK_DGRAM){
      struct sockaddr_in udpsock;
      char * buf;
      buf = (char*) malloc(sizeof(char)*bsize);
      int size = sizeof(sock);
      while (recvfrom(sock, &buf, bsize, 0, (struct sockaddr *)&udpsock, &size) > 0);
      free(buf);
      close(sock);
      exit(0);
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
      free(buffer);
      close(newsock);
      close(sock);
      exit(0);
   }
}

struct cstatus {
   long runs;
   double time;
};
struct cargs {
   int sockfd;
   char *buff;
   int bufsize;
   long num;
   double dur;
   int dat;
};
#define TARG ((struct cargs*) thread_args)
void * client_thread(void *thread_args){
   int s = TARG->sockfd;
   char *b = TARG->buff;
   int bs = TARG->bufsize;
   long n = TARG->num;
   int t = TARG->dur;
   int ds = TARG->dat;

   double start, end;
   if (n == 0){
      start = getTime_usec();
      do{
         if(write(s, b, bs) < 0){
            perror ("Thread write failed");
            exit(1);
         }
         n++;
      } while ((end = getTime_usec()) - start < t);
   }
   else{
      long x;
      start = getTime_usec();
      for (x = n; n > 0; n--){
         if(write(s, b, bs) < 0){
            perror ("Thread write failed");
            exit(1);
         }
      }
      end = getTime_usec();
   }
   TARG->num = n;
   TARG->dur = end-start;

   return (void *) (0);
}
void run_client(int bsize, int protocol, int twrite, char* ip, int duration, int dsize){
   int sock;

   struct sockaddr_in serv;
   sock = socket(AF_INET, protocol, 0);
   serv.sin_family = AF_INET;
   serv.sin_port = htons(PORT);
   char * ipaddr = ip == NULL ? ADDRESS : ip;
   inet_pton(AF_INET, ipaddr, &(serv.sin_addr));

   char *buffer;
   buffer = (char*) malloc(sizeof(char)*bsize);

   if (connect(sock, (struct sockaddr *) &serv, sizeof(serv))< 0){
      perror("Connect Failed\n");
      exit(1);
   }
   printf("Connected to %s on port %d\n", ipaddr, PORT);

   long runs;
   runs = (long) ceil (dsize/bsize);

   printf("Writing data over network\n");
   double start,end;

   duration = duration*1E6;

   struct cargs thd_arg;
   struct cstatus thd_ret;
   pthread_t thread;
   if(twrite){
      thd_arg.sockfd = sock;
      thd_arg.buff = buffer+bsize/2;
      thd_arg.bufsize = bsize;
      thd_arg.num = runs/2;
      thd_arg.dur = duration;
      thd_arg.dat = dsize/2;

      runs = (runs/2.0)+.5;
      dsize = dsize/2;
   }

   if (dsize == 0){
      printf("Time test\n");
      if (twrite){
         if (pthread_create(&thread, NULL, &client_thread, (void*) &thd_arg) <0){
            fprintf(stderr, "Failed to create thread\n");
            exit(1);
         }
      }

      start = getTime_usec();
      do {
         if (write(sock, buffer,bsize) < 0){
            perror("Failed Write\n");
            exit(0);
         }
         runs++;
      }while ((end = getTime_usec()) - start < duration);
   }
   else {
      int i;
      printf("Data test\n");
      if (twrite){
         if (pthread_create(&thread, NULL, &client_thread, (void*) &thd_arg) <0){
            fprintf(stderr, "Failed to create thread\n");
            exit(1);
         }
      }
      start = getTime_usec();
      for (i = runs; i>0; i--){
         if (write(sock, buffer,bsize) < 0){
            perror("Failed Write\n");
            exit(0);
         }
      }
      end = getTime_usec();
   }
   double ttime = 0;
   if (twrite){
      pthread_join(thread, NULL);
      runs += thd_arg.num;
      ttime = thd_arg.dur;
   }
   ttime = (ttime + end - start)/2;
   char* rate = calc_rate(bsize, runs, (ttime)/1E6);
   printf("Write %ld in %f seconds\n", (long)runs*bsize, ttime/1E6);
   printf("Upspeed: %sbits/second\n", rate);
   close(sock);
   free(buffer);
}

int main(int argc, char** argv){
   int i, prot, threaded, bsize, mode, duration;
   unsigned long test_size = 0;
   duration = DURATION;
   char * ip;
   prot = SOCK_STREAM;
   threaded = 0;
   bsize = 1;
   mode = 0;
   ip = NULL;
   for (i=0; i< argc; i++){
      char* ar;
      if ((ar =argv[i])[0] == '-'){
         switch(ar[1]) {
            case 'u': // udp mode
               prot = SOCK_DGRAM;
               break;
            case 'p': // parallel mode
               threaded |= 1;
               break;
            case 'b': // block/buffer size
               bsize = string_to_bytes(ar+2);
               if (!bsize){
                  printf("Invalid size\n%s", USAGE);
                  exit(0);
               }
               break;
            case 's': // server mode
               mode |= 2;
               break;
            case 'c': // client mode
               mode |= 1;
               if (argv[i+1][0] != '-')
                  ip = argv[i+1];
               break;
            case 'd': // data to transfer
               test_size = string_to_bytes(ar+2);
               break;
            case 't': // non-default time parameter
               duration = atoi(argv[i]+2);
               if (duration == 0){
                  fprintf(stderr, "Invalid time parameter\n");
                  exit(1);
               }
               break;
            default:
               printf("Invalid Argument\n%s", USAGE);
               exit(0);
         }
      }
   }
   if (!prot){
      printf("Ping Test\n");
      prot = SOCK_RAW;
   }
   printf("Protocol: %s\n", prot == SOCK_STREAM ? "TCP" : "UDP");
   printf("Threaded: %s\n", threaded ? "Yes": "No");
   printf("Buffer Size: %d\n", bsize);
   int p;
   switch(mode){
      case 1:
         printf("Client Mode\n");
         run_client(bsize, prot, threaded, ip, duration, test_size);
         break;
      case 2:
         printf("Server Mode\n");
         run_server(bsize, prot);
         break;
      default:
         printf("Localhost test\n");
         if ((p = fork()) == 0){
            run_server(bsize, prot);
         }
         sleep(2);
         run_client(bsize, prot, threaded, ip, duration, test_size);
         if (prot == SOCK_STREAM)
            wait(p);
         else
            kill(p, 9);
         break;
   }
   return 0;
}
