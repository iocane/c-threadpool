#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>
#include "../threadpool.h"
#include "test.h"

#define CLOCK_START clock_gettime(CLOCK_MONOTONIC, &start);
#define CLOCK_STOP(m)							\
  clock_gettime(CLOCK_MONOTONIC, &stop);				\
  diff  = timespec_diff(start, stop);					\
  printf("%s: Time: %ld:%ld\n", m, diff.tv_sec, diff.tv_nsec);

int main()
{
  const int num_jobs = 8;
  const int job_size = 1000000000;
  int n;
  int sum = 0;
  struct timespec start, stop, diff;
  struct workd1 w, w1, w2, ws[num_jobs];
  
  struct threadpool *p = threadpool_new(8, 50);
  if(p == NULL) {
    printf("Error creating thread pool\n");
    exit(1);
  }
  
  n = job_size;
  
  /* Single thread */
  w.n = n;
  w.a = set_numbers(w.n);
  CLOCK_START;
  workfn1(&w,0);
  sum = w.r;
  CLOCK_STOP("Single thread");
  printf("Sum: %d\n", sum);

  w1.n = n/2;
  w1.a = w.a;
  w2.n = n - n/2;
  w2.a = w.a + (n/2);
  CLOCK_START;
  threadpool_queue(p, (workfunc)workfn1, (void *)&w1);
  threadpool_queue(p, (workfunc)workfn1, (void *)&w2);
  threadpool_join(p);
  CLOCK_STOP("Thread pool");
  printf("Sum: %d\n", w1.r + w2.r);
  
  pthread_t t1, t2;
  int r;

  CLOCK_START;
  r = pthread_create(&t1, NULL, (workfunc)workfn1, (void *)&w1);
  if(r != 0) {
    perror("Error spawning thread 1");
  }
  r = pthread_create(&t2, NULL, (workfunc)workfn1, (void *)&w2);
  if(r != 0) {
    perror("Error spawning thread 2");
  }
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  CLOCK_STOP("Thread spawn");
  threadpool_delete(p);
  printf("Sum: %d %d\n", w1.r, w2.r);
  
  free(w.a);
  
  return 0;
}

