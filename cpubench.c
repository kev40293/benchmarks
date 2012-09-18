#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

struct thread_info {
  pthread_t tid;
  clockid_t cid;
  void *arg;
  double time;
};
typedef struct thread_info thread_info_t;

double get_clock(clockid_t cid) {
  struct timespec ts;
  if (clock_gettime(cid, &ts) == -1) {
    fprintf(stderr, "Error get clock.\n");
    exit(1);
  }
  return (double) ts.tv_sec + ((double) ts.tv_nsec) / 1e9;
}

void* sumd(void* m) {
  thread_info_t *t = (thread_info_t*)m;
  double n = *(double *)(t->arg), i;
  for (i = 0.0;i < n; i += 1.0);
}

void* sumi(void* m) {
  thread_info_t *t = (thread_info_t*)m;
  long i, n = *(long *)(t->arg);
  for (i = 0;i < n; i++);
}

double run_threads(int num, void *(*func) (void *), void *arg) {
  int j;
  thread_info_t *store = (thread_info_t*) malloc(sizeof(thread_info_t) * num);
  double a = get_clock(CLOCK_PROCESS_CPUTIME_ID);
  
  for (j = 0;j < num;j++) {
    store[j].arg = arg;
    if (pthread_create(&store[j].tid, NULL, func, (void*)&store[j]) != 0) {
      fprintf(stderr, "Error creating thread, quit.\n");
      exit(1);
    }
  }
  
  while (--j >= 0) {
    pthread_join(store[j].tid, NULL);
  }
  free(store);

  return get_clock(CLOCK_PROCESS_CPUTIME_ID) - a;
}

int main() {
  long n = 10000000;
  int i, j, threads[3] = {1,2,4};
  double nd = (double) n, d_total, i_total;
  
  for (i = 0;i < 3;i++) {
    d_total = run_threads(threads[i], sumd, (void*)&nd);
    i_total = run_threads(threads[i], sumi, (void*)&n);
    printf("Threads number: %2d, GFLOPS: %10f, GIOPS: %10f\n", threads[i], 
           (nd * threads[i]) / 1e9 / d_total, (n * threads[i]) / 1e9 / i_total);
  }
  return 0;
}