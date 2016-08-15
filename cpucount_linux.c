/* 
 * Copyright (C) 2016 Thomas Costigliola
 * 
 * Proprietary and confidential, copying is strictly prohibited.
 * 
 */

#include <unistd.h>

int threadpool_cpu_count() {
  int n = (int)sysconf(_SC_NPROCESSORS_ONLN);
  if(n < 0) 
    n = 1;

  return n;
}
