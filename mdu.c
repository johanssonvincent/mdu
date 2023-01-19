#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include "stack.h"
#include "mdu.h"
#include "data.h"
#include "error.h"
#include "thread_funcs.h"

int main(int argc, char *argv[])
{
    /* Initialize start_args struct */
    start_args *sa = init_sa_struct();

    /* Check start arguments */
    check_start_args(argc, argv, sa);

    /* Initialize structs for data and work_order stack */
    init_mutex(sa);
    init_sem(sa);
    sa->data = init_data_structs(sa);
    init_stacks(sa);

    /* Create threads */
    pthread_t thread[sa->n_threads];
    safe_threads(thread, sa);

    /* Wait for threads to finish then join threads and exit */
    join_threads(thread, sa);
    print_results(sa);
    safe_exit(sa);
}

/* ----- Functions -----*/

void check_start_args(int argc, char *argv[], start_args *s)
{
    /* If no dir/file has been specified then add current dir to s->files */
    if (argc == 1)
    {
        s->files[0] = safe_calloc(MAX_LINE, sizeof(char));
        strcpy(s->files[0], ".");
        s->c_files++;
    }
    else
    {
        int flag;

        while ((flag = getopt(argc, argv, ":j:")) != -1)
        {
            switch (flag)
            {
            case 'j':
                s->n_threads = atoi(optarg);
                break;
            case '?':
                fprintf(stderr, "usage: ./mdu [-j THREADS] {FILE/DIR} [FILES]\n");
                exit(errno);
            }
        }

        for (; optind < argc; optind++)
        {
            if (optind > s->n_files)
            {
                realloc_buff(&s->files, s);
            }
            s->files[s->c_files] = safe_calloc(MAX_LINE, sizeof(char));
            strcpy(s->files[s->c_files], argv[optind]);
            s->c_files++;
        }
    }
}

void print_results(start_args *s)
{
    for (int i = 0; i < s->c_files; i++)
    {
        printf("%ld     %s\n", s->data[i]->dir_total, s->files[i]);
    }
}

void safe_exit(start_args *s)
{
    /* Save errno given by threads */
    int exit_no = s->thread_error;

    /* Destroy mutex */
    for (int i = 0; i < 5; i++)
    {
        pthread_mutex_destroy(s->locks[i]);
        free(s->locks[i]);
    }

    /* Free memory & destroy semaphores */
    for (int i = 0; i < s->c_files; i++)
    {
        stack_destroy(s->work_order[i]);
        sem_destroy(s->dir_cond[i]);
        free(s->files[i]);
        free(s->data[i]);
        free(s->dir_cond[i]);
    }
    free(s->work_order);
    free(s->files);
    free(s->data);
    free(s->dir_cond);
    free(s->locks);
    free(s);

    exit(exit_no);
}