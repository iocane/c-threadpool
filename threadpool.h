/* 
 * Copyright (C) 2016 Thomas Costigliola
 */

#include <pthread.h>
#include <semaphore.h>

/* Signature for a function that can be queued to run on the threadppol */
typedef void *(*workfunc)(void *, int);

struct threadpool;

/* Linked list node for list of queued jobs */
struct threadpool_job_node {
  workfunc func;
  void *arg;
  struct threadpool_job_node *next;
};

/* Linked list node for list of worker thread states */
struct threadpool_worker_node {
  struct threadpool *pool;
  pthread_t thread;
  int workerid;
  sem_t sem;
  struct threadpool_worker_node *next;
};

/* Threadpool object */
struct threadpool {
  int queue_size;    /* Maximum number of queued jobs    */
  int queue_count;   /* Current number of queued jobs    */
  int thread_count;  /* Number of active worker threads  */

  /* This semaphore has 1 slot for each slot in the queue. When the
     semaphore hits 0 we know the queue is full, further queueing must
     wait or return with failure */
  sem_t queuesem;

  /* This semaphore controls the join operation. If the queue is not
     empty the join waits on this semaphore and then posts and
     returns. When a worker finishes its job it checks if the queue is
     empty and posts, allowing the join function to continue */
  sem_t joinsem;

  pthread_mutex_t jobsmtx; /* Protects the job list     */
  pthread_mutex_t wkrsmtx; /* Protects the worker list  */
  struct threadpool_worker_node *wn; // next worker node to be scheduled
  struct threadpool_job_node *jt; // top of job queue
  struct threadpool_job_node *jn; // next job 
};

/* Prototype for the worker thread function that gets passed to pthread. */
void * threadpool_worker(void *node);

/**************************************************************************/
/* Threadpool API *********************************************************/
/**************************************************************************/

 /* threadpool_new: create a new threadpool object with 'thread_count' number
  * of OS worker threads and 'queue_size; maximum number of queued jobs.
  */
struct threadpool * threadpool_new(int thread_count, int queue_size);

/* threadpool_delete: safely destroy a threadpool object and free its 
 * resources. Waits for existing jobs to finish.
 */
void threadpool_delete(struct threadpool *p);

/* threadpool_queue: Add a job to threadpool queue, 'func' is called with the
 * parameter 'arg', which is a client maintained object. If the queue is full
 * return immediately with an error
 */
int threadpool_queue(struct threadpool *p, workfunc func, void *arg);

/* threadpool_queue_wait: Add a job to threadpool queue, 'func' is called with
 * the parameter 'arg', which is a client maintained object. If the queue is 
 * full, block the calling thread until a queue space becomes free.
 */
int threadpool_queue_wait(struct threadpool *p, workfunc func, void *arg);

int threadpool_clear_queue(struct threadpool *p);

/* threadpool_join: block the calling thread until all running jobs complete */
int threadpool_join(struct threadpool *p);

int threadpool_cpu_count();
