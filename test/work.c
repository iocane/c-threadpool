#include "test.h"

void * sum_work_values(struct work_data *w)
{
  int sum = 0;
  for(int i = 0; i < w->n; i++) {
    sum += w->a[i];
  }
  w->r = sum;
  return 0;
}
