#include "test.h"

void * workfn1(struct workd1 *w, int id)
{
  int sum = 0;
  for(int i = 0; i < w->n; i++) {
    sum += w->a[i];
  }
  w->r = sum;
  return 0;
}
