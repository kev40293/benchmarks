#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include "wutils.h"

#define NAME "CPUBENCH"

void* sumd(void* m) {
  thread_info_t *t = (thread_info_t*)m;
  double n = *(double *)(t->arg), i;
  for (i = 0.0;i < n; i += 1.0);
  pthread_getcpuclockid(t->tid, &t->cid);
  t->time = get_clock(t->cid);
}

void* sumi(void* m) {
  thread_info_t *t = (thread_info_t*)m;
  long i, n = *(long *)(t->arg);
  for (i = 0;i < n; i++);
  pthread_getcpuclockid(t->tid, &t->cid);
  t->time = get_clock(t->cid);  
}

int main() {
  long n = 100000000;
  int i, j, threads[3] = {1,2,4};
  double nd = (double) n, d_total, i_total;
  
  for (i = 0;i < 3;i++) {
    double **ds = (double**) malloc(sizeof(double*) * threads[i]);
    j = 0;
    while (j < threads[i]) {
      ds[j++] = &nd;
    }
    d_total = run_threads(threads[i], sumd, (void**)ds);
    free(ds);
    //////////////////////////////////////////////////////////
    long **ls = (long**) malloc(sizeof(long*) * threads[i]);
    j = 0;
    while (j < threads[i]) {
      ls[j++] = &n;
    }    
    i_total = run_threads(threads[i], sumi, (void**)ls);
    free(ls);
    
    printf("Threads number: %2d, GFLOPS: %10f, GIOPS: %10f\n", threads[i], 
           (nd * threads[i]) / 1e9 / d_total, (n * threads[i]) / 1e9 / i_total);
  }
  
  printf("%s\n", NAME);
  
  return 0;
}
