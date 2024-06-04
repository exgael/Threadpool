#include "threadpool.h"

#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <unistd.h>
#include <time.h>


// helper function to log active threads
static void log_active_threads(FILE *log_file, int active_threads)
{
    double elapsed_time = (double)clock() / CLOCKS_PER_SEC;
    fprintf(log_file, "%.18e %d\n", elapsed_time, active_threads);
}

// function to increment num active threads in log file
static void log_active_threads_increment(struct pool *pool)
{
    pool->active_threads++;
    log_active_threads(pool->log_file, pool->active_threads);
}

// function to decrement num active threads in log file
static void log_active_threads_decrement(struct pool *pool)
{
    pool->active_threads--;
    log_active_threads(pool->log_file, pool->active_threads);
}

// function to free up resources associated with a thread pool
int threadpool_destroy(struct pool *pool)
{
    // Check for null pointers before attempting to free resources
    if (pool ==            NULL) return 1;
    if (pool->task_list != NULL) list_free(pool->task_list);
    if (pool->threads !=   NULL) free(pool->threads);
    if (pool->args !=      NULL) free(pool->args);
    if (pool->mutex_init)    pthread_mutex_destroy( &(pool->lock     ));
    if (pool->cond_todo_init)pthread_cond_destroy(  &(pool->work_todo));
    if (pool->cond_done_init)pthread_cond_destroy(  &(pool->work_done));
    free(pool);
    return 0;
}

// Worker function to be executed by each thread in the pool
void *worker(void *arg)
{
    // Sleep for a short duration before starting to process tasks
    struct timespec sleep_time = {.tv_sec = 0, .tv_nsec = 10000};
    nanosleep(&sleep_time, NULL);

    // Cast input argument to a worker_arg pointer
    struct worker_arg *w = (struct worker_arg *)arg;
    struct pool *pool = w->pool;

    // Main worker loop
    while (1)
    {
        // Acquire lock before accessing shared resources
        pthread_mutex_lock(&pool->lock);

        // Wait for tasks to be available or the pool to stop running
        while (list_empty(pool->task_list) && pool->running)
        {
            pthread_cond_wait(&pool->work_todo, &pool->lock);
        }

        // Exit the loop if the pool is no longer running and there are no tasks
        if (!pool->running && list_empty(pool->task_list))
        {
            pthread_mutex_unlock(&(pool->lock));
            return NULL;
        }

        // Get the next task from the task list
        struct list_node *node = list_pop_front(pool->task_list);

        // Signal that all tasks are done if the list is now empty and the pool is not running
        if (list_empty(pool->task_list) && !pool->running)
        {
            pthread_cond_signal(&(pool->work_done));
        }

        // If there is a task to process
        if (node)
        {
            // Increase the active threads counter and log the change
            log_active_threads_increment(pool);

            // Release lock before executing task
            __sync_synchronize();
            pthread_mutex_unlock(&(pool->lock));
            struct task *task = (struct task *)node->data;
            task->func(task->arg);
            free(task);
            free(node);

            // Decrease the active threads counter and log the change
            log_active_threads_decrement(pool);
        }
    }
    return NULL; 
}

// Create a thread pool with the specified number of worker threads
struct pool *threadpool_create(int num)
{
    // Validate the number of threads requested
    if (num <= 0)
    {
        perror("Number of thread wanted is invalid!\n");
        return NULL;
    }

    // Allocate memory for the thread pool structure
    struct pool *pool = (struct pool *)malloc(sizeof(struct pool));
    if (pool == NULL)
    {
        perror("Out of memory while creating a new threadpool!\n");
        return NULL;
    }

    // Initialize the task list
    pool->threads = (pthread_t *)malloc(num * sizeof(pthread_t));
    if (pool->threads == NULL)
    {
        perror("Out of memory while creating a table for threads!\n");
        threadpool_destroy(pool);
        return NULL;
    }

    pool->task_list = list_new(NULL, &free_work_item);
    pool->nb_threads = num;
    pool->running = 1;

    
    {   // init mutex and cond variables
        if (pthread_mutex_init(&(pool->lock), NULL) != 0)
        {
            perror("Mutex initiation error!\n");
            threadpool_destroy(pool);
            return NULL;
        }
        pool->mutex_init = 1;

        if (pthread_cond_init(&(pool->work_todo), NULL) != 0)
        {
            perror("pthread_cond work_todo initiation error!\n");
            threadpool_destroy(pool);
            return NULL;
        }
        pool->cond_todo_init = 1;

        if (pthread_cond_init(&(pool->work_done), NULL) != 0)
        {
            perror("pthread_cond work_done initiation error!\n");
            threadpool_destroy(pool);
            return NULL;
        }
        pool->cond_done_init = 1;

        pool->active_threads = 0;
        pool->log_file = fopen("log.txt", "w");
        if (pool->log_file == NULL)
        {
            perror("Error opening log file!\n");
            threadpool_destroy(pool);
            return NULL;
        }
    }

    // Allocate memory for worker arguments
    pool->args = (struct worker_arg *)malloc(num * sizeof(struct worker_arg));
    if (pool->args == NULL)
    {
        perror("Out of memory while creating worker_args!\n");
        threadpool_destroy(pool);
        return NULL;
    }

    // Create and initialize worker threads
    for (int i = 0; i < num; ++i)
    {
        pool->args[i].pool = pool;
        pool->args[i].id = i;

        if (pthread_create(&(pool->threads[i]), NULL, worker, &(pool->args[i])))
        {
            perror("Thread initiation error!\n");
            threadpool_destroy(pool);
            return NULL;
        }
    }
    return pool;
}

// Add a task to the thread pool
void threadpool_add_task(struct pool *pool, func_t function, void *arg)
{
    // Check if the pool is running before adding a task
    if (!pool->running)
    {
        perror("Error, adding task to a non running pool!\n");
        threadpool_destroy(pool);
        return;
    }

    // Allocate memory for the task structure
    struct task *task = (struct task *)malloc(sizeof(struct task));

    if (task == NULL)
    {
        perror("Out of memory while creating a task!\n");
        threadpool_destroy(pool);
        return;
    }

    task->func = function;
    task->arg = arg;

    // Create a new list node for the task
    struct list_node *node = list_node_new(task);

    if (node == NULL)
    {
        perror("Out of memory while creating a task!\n");
        threadpool_destroy(pool);
        return;
    }

    // Add the task to the task list and signal a worker thread
    pthread_mutex_lock(&(pool->lock));
    list_push_back(pool->task_list, node);
    pthread_cond_signal(&(pool->work_todo));
    pthread_mutex_unlock(&(pool->lock));
}

// Wait for all tasks in the pool to complete and free resources
void threadpool_join(struct pool *pool)
{
    // Set the running flag to 0, indicating that the pool is shutting down
    pool->running = 0;

    // Acquire the lock before modifying shared resources
    pthread_mutex_lock(&pool->lock);
    pthread_cond_broadcast(&pool->work_todo);
    pthread_mutex_unlock(&pool->lock);

    // Wait for all worker threads to finish
    for (int i = 0; i < pool->nb_threads; ++i)
    {
        pthread_join(pool->threads[i], NULL);
    }

    // Free all resources associated with the pool
    threadpool_destroy(pool);
}
