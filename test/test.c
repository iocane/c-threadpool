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
  printf("%s: Time: %ld.%ld\n", m, diff.tv_sec, diff.tv_nsec);

int main()
{
  const int num_jobs = 8;
  const int job_size = 1000000000;
  int n;
  int sum = 0;
  struct timespec start, stop, diff;
  struct work_data work_single_thread, work_part_1, work_part_2, ws[num_jobs];

  struct threadpool *p = threadpool_new(8, 50);
  if(p == NULL) {
    printf("Error creating thread pool\n");
    exit(1);
  }

  n = job_size;

  /* Single thread */
  work_single_thread.n = n;
  work_single_thread.a = set_numbers(work_single_thread.n);

  CLOCK_START;
  sum_work_values(&work_single_thread);
  sum = work_single_thread.r;
  CLOCK_STOP("Single thread");
  printf("Sum: %d\n", sum);

  /* Warning! assuming even sized array */
  work_part_1.n = n/2;
  work_part_1.a = work_single_thread.a;
  work_part_2.n = n/2;
  work_part_2.a = work_single_thread.a + n/2;

  CLOCK_START;
  threadpool_queue(p, (workfunc)sum_work_values, (void *)&work_part_1);
  threadpool_queue(p, (workfunc)sum_work_values, (void *)&work_part_2);
  threadpool_join(p);
  CLOCK_STOP("Thread pool");
  printf("Sum: %d\n", work_part_1.r + work_part_2.r);

  pthread_t t1, t2;
  int r;

  CLOCK_START;
  r = pthread_create(&t1, NULL, (void * (*)(void *))sum_work_values, (void *)&work_part_1);
  if(r != 0) {
    perror("Error spawning thread 1");
  }
  r = pthread_create(&t2, NULL, (void * (*)(void *))sum_work_values, (void *)&work_part_2);
  if(r != 0) {
    perror("Error spawning thread 2");
  }
  pthread_join(t1, NULL);
  pthread_join(t2, NULL);
  CLOCK_STOP("Thread spawn");
  threadpool_delete(p);
  printf("Sum: %d\n", work_part_1.r + work_part_2.r);

  free(work_single_thread.a);

  return 0;
}
