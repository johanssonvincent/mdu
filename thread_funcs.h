/**
 * @author  Vincent Johansson
 * @since   2021-01-03
 *
 */

#ifndef THREAD_FUNCS_H
#define THREAD_FUNCS_H

#include "mdu.h"
#include "error.h"

/**
 * @brief start threads safely
 *
 * @param thread array with all pthread_t
 * @param s start_args struct
 */
void safe_threads(pthread_t thread[], start_args *s);

/**
 * @brief join all threads safely
 *
 * @param thread array with all pthread_t
 * @param s start_args struct
 */
void join_threads(pthread_t thread[], start_args *s);

/**
 * @brief checks for work in stack using mutex lock to avoid thread racing
 *
 * @param s start_args struct
 * @param mutex pointer to the mutex lock that will be used
 * @param str pointer to a pointer of a string, used to save file/dir name to be checked
 * @return true if work available otherwise false
 */
bool work_available(Stack *s, pthread_mutex_t *mutex, char **str);

/**
 * @brief checks if any thread is currently working in a directory
 *
 * @param s start_args struct
 * @param mutex pointer to the mutex lock that will be used
 * @param count integer specifying which given file/dir is currently being worked on
 * @return true if there are active workers otherwise false
 */
bool active_worker(start_args *s, pthread_mutex_t *mutex, int count);

/**
 * @brief increase or decrease amount of active workers for a directory
 *
 * @param active_workers pointer to integer with amount of active workers
 * @param mutex mutex lock
 * @param value marks if increase or decrease, -1 to decrease or 1 to increase
 */
void mark_active_work(int *active_workers, pthread_mutex_t *mutex, int value);

/**
 * @brief handles all work collecting data from files and directories
 *
 * @param s start_args struct
 * @param str name of file/dir to execute work on
 * @param count integer specifying which given file/dir is currently being worked on
 */
void do_work(start_args *s, char *str, int count);

/**
 * @brief Function used when starting threads
 *
 * @param arg
 * @return void*
 */
void *work_func(void *arg);

#endif /* THREAD_FUNCS_H */