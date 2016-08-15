/* Utility functions */
int * set_numbers(int n);
struct timespec timespec_diff(struct timespec start, struct timespec end);

/* Worker arguments and functions */

struct workd1 {
  int n;  /* count  */
  int *a; /* array  */
  int r;  /* result */
};
void * workfn1(struct workd1 *w, int id);

struct workd2 {
  
};
void * workfn2(struct workd2 *w, int id);
