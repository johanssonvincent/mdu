/**
 * @file mdu.c
 * @author Vincent Johansson (dv14vjn@cs.umu.se)
 * @brief
 * @version 0.1
 * @date 2022-10-31
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#define MAX_LINE 1024

typedef struct data
{
    DIR *dir;
    struct dirent *dirp;
    blkcnt_t dir_total;
} data;

typedef struct start_args
{
    data *data;
    char **files;
    bool work_done;
    int n_threads;
    int n_files;
    int c_files;
    int c_dir;
} start_args;

/* ----- Function declaration -----*/
void *init_sa_struct(void);
void *init_data_struct(start_args *s);
void check_start_args(int argc, char *argv[], start_args *s);

void safe_thread(pthread_t thread, start_args *s);
void join_threads(pthread_t thread[], start_args *s);
void *thread_func(void *arg);

void *safe_calloc(int amount, size_t size);
void realloc_buff(char ***buffer, start_args *s);
void safe_exit(start_args *s);

int main(int argc, char *argv[])
{
    /* Initialize start_args struct */
    start_args *sa = init_sa_struct();

    /* Check start arguments */
    check_start_args(argc, argv, sa);

    /* Initialize structs for data */
    sa->data = init_data_struct(sa);

    /* Create threads */
    pthread_t thread[sa->n_threads];

    for (int i = 0; i < sa->n_threads; i++)
    {
        safe_thread(thread[i], sa);
    }

    // join_threads(thread, sa);

    safe_exit(sa);
}

/* ----- Functions -----*/

/**
 * @brief Initialize start_args struct
 *
 * @return void* pointer to allocated memory
 */
void *init_sa_struct(void)
{
    /* Allocate memory for struct */
    start_args *s;
    s = safe_calloc(1, sizeof(start_args));

    /* Initialize values */
    s->work_done = false;
    s->n_threads = 1;
    s->n_files = 50;
    s->c_files = 0;
    s->c_dir = 0;

    /* Allocate memory for buffers */
    s->files = safe_calloc(s->n_files, sizeof(char *));

    return s;
}

/**
 * @brief Initialize data struct
 *
 * @param s start_args struct
 * @return void* pointer to allocated memory
 */
void *init_data_struct(start_args *s)
{
    /* Allocate memory */
    data *d;
    d = safe_calloc(s->c_files, sizeof(data));

    /* Set initial value for dir_total to 0 */
    d->dir_total = 0;

    return d;
}

/**
 * @brief Check arguments given by user with getopt
 *
 * @param argc      argc
 * @param argv      argv
 * @param s         start_args struct
 */
void check_start_args(int argc, char *argv[], start_args *s)
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

void safe_thread(pthread_t thread, start_args *s)
{
    if (pthread_create(&thread, NULL, thread_func, s) != 0)
    {
        perror(strerror(errno));
    }
}

void join_threads(pthread_t thread[], start_args *s)
{
    for (int i = 0; i < s->n_threads; i++)
    {
        if (pthread_join(thread[i], NULL) != 0)
        {
            perror(strerror(errno));
        }
    }
}

void *thread_func(void *arg)
{
    start_args *s = (start_args *)arg;

    while ()
        while ((s->data[] = readdir(d->dir)) != NULL)
        {
        }
    return 0;
}

/**
 * @brief Safe usage of calloc function
 *
 * @param size      size of memory to be allocated
 * @return void*    pointer to allocated memory
 */
void *safe_calloc(int amount, size_t size)
{
    void *mem_block;

    if ((mem_block = calloc(amount, size)) == NULL)
    {
        perror(strerror(errno));
        exit(errno);
    }

    return mem_block;
}

/**
 * @brief Reallocate **char buffer.
 *
 * @param buffer  	Pointer to **char buffer.
 * @param s			start_args struct.
 */
void realloc_buff(char ***buffer, start_args *s)
{
    char **temp;

    s->n_files *= 2;

    temp = realloc(*buffer, sizeof(char *) * s->n_files);
    if (temp == NULL)
    {
        perror(strerror(errno));
        exit(errno);
    }
    *buffer = temp;

    for (int i = 0; i < s->n_files; i++)
    {
        char *temp2;
        temp2 = realloc(*buffer[i], sizeof(char) * MAX_LINE);
        if (temp2 == NULL)
        {
            perror(strerror(errno));
            exit(errno);
        }
        else
        {
            *buffer[i] = temp2;
        }
    }
}

/**
 * @brief Used to free allocated memory before program exit
 *
 * @param s struct containing start arguments
 */
void safe_exit(start_args *s)
{
    for (int i = 0; i < s->c_files; i++)
    {
        free(s->files[i]);
    }
    free(s->files);
    free(s);
}