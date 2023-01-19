/**
 * @author Vincent Johansson (dv14vjn@cs.umu.se)
 * @date 2023-01-19
 */

#ifndef MDU_H
#define MDU_H

#include <semaphore.h>
#include "stack.h"

#define MAX_LINE 1024

/**
 * @brief Struct containing the size of a dir and how many threads are working in the dir
 *
 */
typedef struct data
{
    long int dir_total;
    int active_work;
} data;

/**
 * @brief Struct containing all data being sent to the threads
 *
 */
typedef struct start_args
{
    Stack **work_order;
    data **data;
    char **files;
    sem_t **dir_cond;
    pthread_mutex_t **locks;
    int n_threads;
    int n_files;
    int c_files;
    int thread_error;
} start_args;

/**
 * @brief Check arguments given by user with getopt
 *
 * @param argc      argc
 * @param argv      argv
 * @param s         start_args struct
 */
void check_start_args(int argc, char *argv[], start_args *s);

/**
 * @brief Prints the results
 *
 * @param s start_args struct
 */
void print_results(start_args *s);

/**
 * @brief Free allocated memory and destroy mutex locks before program exit
 *
 * @param s struct containing start arguments
 * @param locks[] mutex locks to be destroyed
 */
void safe_exit(start_args *s);

#endif /* MDU_H */