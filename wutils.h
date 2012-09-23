#ifndef WUTILS_H
#define WUTILS_H

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

void** run_threads_init_arg(void* arg) {

}

double run_threads(int num, void *(*func) (void *), void **arg) {
  int j;
  thread_info_t *store = (thread_info_t*) malloc(sizeof(thread_info_t) * num);

  double a = get_clock(CLOCK_REALTIME);  
  for (j = 0;j < num;j++) {
    store[j].arg = arg[j];
    if (pthread_create(&store[j].tid, NULL, func, (void*)&store[j]) != 0) {
      fprintf(stderr, "Error creating thread, quit.\n");
      exit(1);
    }
  }
  for (j = 0; j < num; j++) {
    pthread_join(store[j].tid, NULL);
  }
  double b = get_clock(CLOCK_REALTIME) - a;
  free(store);

  return b;
}

long strtob(const char *str) {
  long b = atoi(str);
  if (b <= 0) return 0L;
  char c = str[strlen(str)-1];
  switch (c) {
    case 'K' : b <<= 10; break;
    case 'M' : b <<= 20; break;
    case 'G' : b <<= 30; break;
    default  : break;
  }
  return b;
}

#endif