#include <stdlib.h>
#include "test.h"

int * set_numbers(int n)
{
  int *a = (int *)calloc(n, sizeof(int));
  
  for(int i = 0; i < n; i++) {
    a[i] = 1;
  }

  return a;
}

struct timespec timespec_diff(struct timespec start, struct timespec end)
{
	struct timespec diff;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		diff.tv_sec = end.tv_sec-start.tv_sec-1;
		diff.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		diff.tv_sec = end.tv_sec-start.tv_sec;
		diff.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return diff;
}
