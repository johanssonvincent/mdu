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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>

#define MAX_LINE 1024

typedef struct data_struct
{
    DIR *dir;
    struct dirent *dirp;

    char **files;
    int n_threads;
    int n_files;
    int c_files;
} data_struct;

/* ----- Function declaration -----*/
void *init_struct(void);
void check_data_struct(int argc, char *argv[], data_struct *s);

void start_threads(pthread_t threads[], data_struct *s);
void join_threads(pthread_t thread[], data_struct *s);
void *thread_func(void *arg);

void *safe_calloc(size_t size);
void realloc_buff(char ***buffer, data_struct *s);
void safe_exit(data_struct *s);

int main(int argc, char *argv[])
{
    /* Initialize data_struct struct */
    data_struct *ds = init_struct();

    /* Check start arguments */
    check_data_struct(argc, argv, ds);

    /* Create threads */
    pthread_t thread[ds->n_threads];

    start_threads(thread, ds);

    join_threads(thread, ds);

    safe_exit(ds);
}

/* ----- Functions -----*/

/**
 * @brief Initialize a struct
 *
 * @return void* pointer to allocated memory
 */
void *init_struct(void)
{
    /* Allocate memory for struct */
    data_struct *ds;
    ds = safe_calloc(sizeof(data_struct));

    /* Initialize values */
    ds->n_threads = 1;
    ds->n_files = 50;
    ds->c_files = 0;

    /* Allocate memory for buffers */
    ds->files = safe_calloc(sizeof(char *) * ds->n_files);

    return ds;
}

/**
 * @brief Check arguments given by user with getopt
 *
 * @param argc      argc
 * @param argv      argv
 * @param s         data_struct struct
 */
void check_data_struct(int argc, char *argv[], data_struct *s)
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
        s->files[s->c_files] = safe_calloc(sizeof(char) * MAX_LINE);
        strcpy(s->files[s->c_files], argv[optind]);
        s->c_files++;
    }
}

void start_threads(pthread_t thread[], data_struct *s)
{
    for (int i = 0; i < s->n_threads; i++)
    {
        pthread_create(&thread[i], NULL, thread_func, s);
    }
}

void join_threads(pthread_t thread[], data_struct *s)
{
    for (int i = 0; i < s->n_threads; i++)
    {
        pthread_join(thread[i], NULL);
    }
}

void *thread_func(void *arg)
{
    data_struct *s = (data_struct *)arg;
    struct stat stat_file;

    s->dir = opendir(s->files[0]);

    if (s->dir == NULL && errno == ENOTDIR)
    {
        lstat(s->files[0], &stat_file);
        printf("%ld %s", stat_file.st_blocks, s->files[0]);
    }

    return 0;
}

/**
 * @brief Safe usage of calloc function
 *
 * @param size      size of memory to be allocated
 * @return void*    pointer to allocated memory
 */
void *safe_calloc(size_t size)
{
    void *mem_block;

    if ((mem_block = calloc(1, size)) == NULL)
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
 * @param s			data_struct struct.
 */
void realloc_buff(char ***buffer, data_struct *s)
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
void safe_exit(data_struct *s)
{
    for (int i = 0; i < s->c_files; i++)
    {
        free(s->files[i]);
    }
    free(s->files);
    free(s);
}