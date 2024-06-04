#ifndef INF3170_THREADPOOL_H_
#define INF3170_THREADPOOL_H_

#include <pthread.h>
#include <stdint.h>
#include <sys/types.h>
#include <stdio.h>
#include "processing.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void *(*func_t)(void *);

struct worker_arg {
  int id;
  struct pool *pool;
};

struct task {
  func_t func;
  void *arg;
};

struct pool {
  int nb_threads;
  pthread_t *threads;
  struct worker_arg *args;
  pthread_barrier_t ready;

  pthread_mutex_t lock;
  pthread_cond_t work_todo;
  pthread_cond_t work_done;

  int mutex_init;
  int cond_todo_init;
  int cond_done_init;
  int active_threads;
  FILE *log_file;

  struct list *task_list;

  int running;
};

struct pool *threadpool_create(int num);
void threadpool_add_task(struct pool *pool, func_t function, void *arg);
void threadpool_join(struct pool *pool);
int threadpool_destroy(struct pool *pool);

#ifdef __cplusplus
}
#endif

#endif /* INF3170_THREADPOOL_H_ */