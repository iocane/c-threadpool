/* Utility functions */
int * set_numbers(int n);
struct timespec timespec_diff(struct timespec start, struct timespec end);

/* Worker arguments and functions */

struct work_data {
  int n;  /* count  */
  int *a; /* array  */
  int r;  /* result */
};
void * sum_work_values(struct work_data *w);
