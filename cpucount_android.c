/* 
 * Copyright (C) 2016 Thomas Costigliola
 * 
 * Proprietary and confidential, copying is strictly prohibited.
 * 
 */

#include <cpu-features.h>

int threadpool_cpu_count() {
  return (int)android_getCpuCount();
}
