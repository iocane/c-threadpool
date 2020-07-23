/* 
 * Copyright (C) 2016 Thomas Costigliola
 */

#include <stdlib.h>
#include <stdbool.h>
#include "threadpool.h"

struct threadpool * threadpool_new(int threadcount, int queuesize) {
  if(threadcount < 1 || queuesize < 1 || queuesize < threadcount) {
    /* Do not allow empty pools and queues smaller than the number of threads */
    return NULL;
  }

  struct threadpool *p = (struct threadpool *)calloc(1, sizeof(struct threadpool));
  p->threadcount = threadcount;
  p->queuesize = queuesize;
  p->queuecount = 0;
  
  sem_init(&p->queuesem, 0, p->queuesize);
  sem_init(&p->joinsem, 0, 1);

  pthread_mutex_init(&p->jobsmtx, NULL);
  pthread_mutex_init(&p->wkrsmtx, NULL);

  // create the first worker thread node
  p->wn = (struct threadpool_worker_node *)calloc(1, sizeof(struct threadpool_worker_node));
  p->wn->pool = p;
  p->wn->next = p->wn;
  p->wn->workerid = 0;
  sem_init(&p->wn->sem, 0, 0);
  pthread_create(&p->wn->thread, NULL, threadpool_worker, (void *)p->wn);
  
  // create additional worker thread nodes
  struct threadpool_worker_node *w;
  for(int i = 1; i < threadcount; i++) {
    w = (struct threadpool_worker_node *)calloc(1, sizeof(struct threadpool_worker_node));
    w->pool = p;
    w->next = p->wn->next;
    w->workerid = i;
    p->wn->next = w;
    sem_init(&w->sem, 0, 0);
    pthread_create(&w->thread, NULL, threadpool_worker, (void *)w);
  }
  
  // create first job node
  p->jt = p->jn = (struct threadpool_job_node *)calloc(1, sizeof(struct threadpool_job_node));
  p->jt->func = NULL;
  p->jt->arg = NULL;
  p->jt->next = NULL;
  
  // create additional job nodes
  struct threadpool_job_node *j;
  for(int i = 1; i < queuesize; i++) {
    j = (struct threadpool_job_node *)calloc(1, sizeof(struct threadpool_job_node));
    j->func = NULL;
    j->arg = NULL;
    j->next = p->jt->next;
    p->jt->next = j;
  }
  
  return p;
}

void threadpool_delete(struct threadpool *p) {
  struct threadpool_job_node *j;
  struct threadpool_worker_node *w;
    
  // post a null job for each worker
  for(int i = 0; i < p->threadcount; i++) {
    threadpool_queue_wait(p, NULL, NULL);
  }
  
  // wait for all workers to terminate
  for(int i = 0; i < p->threadcount; i++) {
    pthread_join(p->wn->thread, NULL);
    p->wn = p->wn->next;
  }
  
  // free the job nodes
  j = p->jt;
  for(int i = 0; i < p->queuesize; i++) {
    j = j->next;
    free(p->jt);
    p->jt = j;
  }
  
  // free the worker thread nodes
  w = p->wn;
  for(int i = 0; i < p->threadcount; i++) {
    w = w->next;
    sem_destroy(&p->wn->sem);
    free(p->wn);
    p->wn = w;
  }
  
  sem_destroy(&p->queuesem);
  sem_destroy(&p->joinsem);

  free(p);
}

void * threadpool_worker(void *node) {
  struct threadpool_worker_node *n = (struct threadpool_worker_node *)node;
  struct threadpool *p = n->pool;
  struct threadpool_job_node *j;
  bool alive = true;
  workfunc func;
  void *arg;
  
  while(alive) {

    /* This semaphore starts at 0, the queuing function will post on
       it to activate this worker thread allowing the thread to
       advance and decrementing the semaphore back to 0. When its job
       is finished it waits again on the null semaphore unless it has
       already been queued again. */
    sem_wait(&n->sem);
    
    /* Lock the job queue and select the next job */
    pthread_mutex_lock(&p->jobsmtx);

    func = p->jt->func;
    arg = p->jt->arg;
    
    j = p->jt->next;
    p->jt->next = p->jn->next;
    p->jn->next = p->jt;
    p->jt = j;

    pthread_mutex_unlock(&p->jobsmtx);
    
    sem_post(&p->queuesem);
    
    if(func == NULL) {
      alive = false;
    } else {
      func(arg, n->workerid);
    }
    
    pthread_mutex_lock(&p->jobsmtx);
    p->queuecount--;
    if(p->queuecount == 0) {
      /* Wakeup a waiting join operation */
      sem_post(&p->joinsem);
    }
    pthread_mutex_unlock(&p->jobsmtx); 
  }
  
  return NULL;
}

/* Add a job to work queue and return:
  0 on success, -1 if the queue is full */
int threadpool_queue(struct threadpool *p, workfunc func, void *arg) {
  /* Wait for the job queue to be writeable */
  pthread_mutex_lock(&p->jobsmtx);
  if(p->jn == NULL) {
    // the jobe queue is full
    pthread_mutex_unlock(&p->jobsmtx);
    return -1;
  }
  /* We know this won't block since we verified there are slots in the
     queue */
  sem_wait(&p->queuesem);
  if(p->queuecount == 0) {
    /* The queue was empty, make sure joins block now until it becomes
       empty again */
    sem_wait(&p->joinsem);
  }
  p->queuecount++;
  p->jn->func = func;
  p->jn->arg = arg;
  p->jn = p->jn->next;
  pthread_mutex_unlock(&p->jobsmtx);

  pthread_mutex_lock(&p->wkrsmtx);
  sem_post(&p->wn->sem);
  p->wn = p->wn->next;
  pthread_mutex_unlock(&p->wkrsmtx);
  
  return 0;
}

int threadpool_queue_wait(struct threadpool *p, workfunc func, void *arg) {
  /* Wait on a free queue slot before continuing */
  sem_wait(&p->queuesem);
  
  /* Wait for the job queue to be writeable */
  pthread_mutex_lock(&p->jobsmtx);
  if(p->queuecount == 0) {
    /* The queue was empty, make sure joins block now until it becomes
       empty again */
    sem_wait(&p->joinsem);
  }
  p->queuecount++;
  p->jn->func = func;
  p->jn->arg = arg;
  p->jn = p->jn->next;
  pthread_mutex_unlock(&p->jobsmtx);
  
  pthread_mutex_lock(&p->wkrsmtx);
  sem_post(&p->wn->sem);
  p->wn = p->wn->next;
  pthread_mutex_unlock(&p->wkrsmtx);
  
  return 0;
}

int threadpool_join(struct threadpool *p) {
  sem_wait(&p->joinsem);
  sem_post(&p->joinsem);
  return 0;
}
