/**
 * @file mdu.c
 * @author Vincent Johansson (dv14vjn@cs.umu.se)
 * @brief
 * @version 0.1
 * @date 2022-11-23
 *
 * @copyright Copyright (c) 2022
 *
 */

#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <dirent.h>
#include <errno.h>
#include "stack.h"

#define MAX_LINE 1024

typedef struct data
{
    long int dir_total;
} data;

typedef struct start_args
{
    Stack **work_order;
    data **data;
    char **files;
    int n_threads;
    int n_files;
    int c_files;
} start_args;

sem_t lock[5];

/* ----- Function declaration -----*/
void *init_sa_struct(void);
void *init_data_structs(start_args *s);
void init_sem(sem_t lock[]);
void init_stacks(start_args *s);
void init_stack(start_args *s);
void check_start_args(int argc, char *argv[], start_args *s);
void safe_threads(pthread_t thread[], start_args *s);
void join_threads(pthread_t thread[], start_args *s);
void do_work(start_args *s, char *str, int count);
bool new_dir(char *str);
void *work_func(void *arg);
void print_results(start_args *s);
void *safe_calloc(int amount, size_t size);
void realloc_buff(char ***buffer, start_args *s);
void safe_exit(start_args *s, sem_t lock[]);

int main(int argc, char *argv[])
{
    /* Initialize start_args struct */
    start_args *sa = init_sa_struct();

    /* Check start arguments */
    check_start_args(argc, argv, sa);

    /* Initialize structs for data and work_order stack */
    sa->data = init_data_structs(sa);
    init_stacks(sa);

    /* Create threads and initialize semaphores*/
    pthread_t thread[sa->n_threads + 1];
    init_sem(lock);
    safe_threads(thread, sa);

    /* Wait for threads to finish then join threads and exit */
    sem_wait(&lock[3]);
    join_threads(thread, sa);
    print_results(sa);
    safe_exit(sa, lock);
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
    s->n_threads = 1;
    s->n_files = 50;
    s->c_files = 0;

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
void *init_data_structs(start_args *s)
{
    /* Allocate memory */
    data **d;
    d = safe_calloc(s->c_files, sizeof(data *));

    for (int i = 0; i < s->c_files; i++)
    {
        d[i] = safe_calloc(1, sizeof(data));
        d[i]->dir_total = 0;
    }

    return d;
}

void init_sem(sem_t lock[])
{
    /* Initialize variable locks */
    for (int i = 0; i < 3; i++)
    {
        sem_init(&lock[i], 0, 1);
    }

    /* Semaphore to track if all threads are done working */
    sem_init(&lock[3], 0, 0);
}

void init_stacks(start_args *s)
{
    s->work_order = safe_calloc(s->c_files, sizeof(Stack **));
    for (int i = 0; i < s->c_files; i++)
    {
        s->work_order[i] = stack_create();
        stack_push(s->work_order[i], s->files[i]);
    }
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
    /* If no dir/file has been specified then add current dir to s->files */
    if (argc == 1)
    {
        s->files[0] = safe_calloc(MAX_LINE, sizeof(char));
        strcpy(s->files[0], "./");
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

void safe_threads(pthread_t thread[], start_args *s)
{
    /* Start work threads */
    for (int i = 0; i < s->n_threads; i++)
    {
        if (pthread_create(&thread[i], NULL, work_func, s) != 0)
        {
            perror(strerror(errno));
        }
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

void do_work(start_args *s, char *str, int count)
{
    struct dirent *ent_dir;
    DIR *dir;
    struct stat file_stat;
    unsigned char type;
    long int local_total = 0;
    char *tmp_str;

    if ((dir = opendir(str)) == NULL)
    {
        if (errno == ENOTDIR)
        {
            lstat(str, &file_stat);
            local_total += file_stat.st_blocks;
            sem_wait(&lock[2]);
            s->data[count]->dir_total += local_total;
            sem_post(&lock[2]);
            return;
        }
        else if (errno == ENOENT)
        {
            return;
        }
        else
        {
            perror(strerror(errno));
            return;
        }
    }

    while ((ent_dir = readdir(dir)) != NULL)
    {
        type = ent_dir->d_type;
        switch (type)
        {
        case DT_DIR:
            tmp_str = safe_calloc(strlen(ent_dir->d_name) + 1, sizeof(char));
            strcpy(tmp_str, ent_dir->d_name);
            sem_wait(&lock[1]);
            stack_push(s->work_order[count], tmp_str);
            sem_post(&lock[1]);
            free(tmp_str);
            break;
        case DT_REG:
            lstat(ent_dir->d_name, &file_stat);
            local_total += file_stat.st_blocks;
            break;
        }
    }
    sem_wait(&lock[2]);
    s->data[count]->dir_total += local_total;
    sem_post(&lock[2]);

    closedir(dir);
    return;
}

/**
 * @brief
 *
 * @param arg
 * @return void*
 */
void *work_func(void *arg)
{
    start_args *s = (start_args *)arg;

    char *current;

    for (int i = 0; i < s->c_files; i++)
    {
        sem_wait(&lock[0]);
        while (!stack_is_empty(s->work_order[i]))
        {
            current = stack_pop(s->work_order[i]);
            sem_post(&lock[0]);
            do_work(s, current, i);
            free(current); /* Free memory of used string */
            sem_wait(&lock[0]);
        }
        sem_post(&lock[0]);
    }
    sem_post(&lock[3]);
    return 0;
}

void print_results(start_args *s)
{
    for (int i = 0; i < s->c_files; i++)
    {
        printf("%ld     %s\n", s->data[i]->dir_total, s->files[i]);
    }
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
 * @brief Free allocated memory and destroy semaphores before program exit
 *
 * @param s struct containing start arguments
 */
void safe_exit(start_args *s, sem_t lock[])
{
    /* Free memory */
    for (int i = 0; i < s->c_files; i++)
    {
        stack_destroy(s->work_order[i]);
        free(s->files[i]);
        free(s->data[i]);
    }
    free(s->work_order);
    free(s->files);
    free(s->data);
    free(s);

    /* Destroy all semaphores */
    for (int i = 0; i < 4; i++)
    {
        sem_destroy(&lock[i]);
    }
}