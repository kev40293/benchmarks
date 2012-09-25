#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define MOD(a,b) ((a) & ((b) - 1))
#define RUP(a,b) ((a) + ((-(a)) & ((b)-1)))
#define SEQ_ACC 1
#define RND_ACC 2
#define MEM_CPY 3

#define MCPY_CAPA 16
#define SEQ_CAPA 32
#define RND_CAPA 32

#define NAME "MEMBENCH"

struct targ {
  long processed;
  long size;
  char* store;
};
typedef struct targ targ_t;
#include "wutils.h"

void* bm_mem_cpy(void* thread) {
  targ_t *a = (targ_t*) (((thread_info_t*)thread)->arg);
  char *src = a->store;
  int i = 0;
  long* m = (long*) malloc(sizeof(long) * MCPY_CAPA);
  while (i < MCPY_CAPA) m[i] = a->size * (i++);

  i = 0;
  while (i < 10000) {
    memcpy(a->store + m[MOD(i + 1, MCPY_CAPA)], a->store + m[MOD(i, MCPY_CAPA)], a->size);
    i++;
  }
  a->processed = a->size * 10000;
  free(m);
}

void* bm_seq_acc(void* thread) {
  targ_t *a = (targ_t*) (((thread_info_t*)thread)->arg);
  int i = 0; 
  long k = 0, j = a->size * (SEQ_CAPA - 1);

  while (i++ < 100000) {
    k = 0;
    while (k < j) {
      k = RUP(k, a->size);
      a->store[k++];
    }
  }
  a->processed = SEQ_CAPA * 100000;
}

void* bm_rnd_acc(void* thread) {
  targ_t *a = (targ_t*) (((thread_info_t*)thread)->arg);
  int i = 0, k;
  long* m = (long*) malloc(sizeof(long) * RND_CAPA);
  while (i < MCPY_CAPA) m[i] = a->size * (i++);  
  
  i = 0;
  while (i++ < 100000) {
    k = 0;
    while (k++ < RND_CAPA) {
      a->store[m[MOD(rand(), RND_CAPA)]];
    }
  }
  a->processed = RND_CAPA * 100000;
  free(m);
}

int main(int argc, char **argv) {
  int c = 0, threads_num = 1, mode = SEQ_ACC;
  long block_size = 0;
  
  while ((c = getopt(argc, argv, "m:t:b:")) != -1) {
    switch (c) {
      case 'm' :
        mode = atoi(optarg);
        break;
      case 't' :
        threads_num = atoi(optarg);
        break;
      case 'b' :
        block_size = strtob(optarg);
        break;
      default :
        fprintf(stderr, "Invalid option.\n");
        return 1;
    }
  }
  
  void *(*func) (void *);
  switch (mode) {
    case MEM_CPY :
      func = &bm_mem_cpy;
      break;
    case SEQ_ACC :
      func = &bm_seq_acc;
      break;
    case RND_ACC :
      func = &bm_rnd_acc;
      break;          
    default : return 1;
  }  
  
  int i, j;
  targ_t **space = (targ_t**) malloc(threads_num * sizeof(targ_t*));
  for (j = 0;j < threads_num;j++) {
    targ_t* arg = (targ_t*) malloc(sizeof(targ_t));
    arg->processed = 0;
    arg->size = block_size;
    switch (mode) {
      case MEM_CPY :
        arg->store = malloc(arg->size * MCPY_CAPA);
        break;
      case SEQ_ACC :
        arg->store = malloc(arg->size * SEQ_CAPA);
        break;
      case RND_ACC :
        arg->store = malloc(arg->size * RND_CAPA);
        break;          
      default : return 1;
    }
    space[j] = arg;
  }
  double t = run_threads(threads_num, func, (void**)space);
  long processed = 0;
  for (j = 0;j < threads_num;j++) {
    processed += space[j]->processed;
  }
  printf("Threads: %d, Throughput: %.2fMB/s, Duration: %.2fms.\n", threads_num, (double) processed / 1024 / 1024 / t, t * 1e3);
  
  //free
  for (j = 0;j < threads_num;j++) {
    free(space[j]->store);
    free(space[j]);
  }
  free(space);
  
  printf("%s,%ld,%d,%d,%f\n", NAME, block_size, threads_num, mode, (double) processed / 1024 /1024 /t);
  
  return 0;
}
