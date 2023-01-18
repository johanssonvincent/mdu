/**
 * @file mdu.c
 * @author Vincent Johansson (dv14vjn@cs.umu.se)
 * @date 2023-01-18
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
    int active_work;
} data;

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

/* ----- Function declaration -----*/
void *init_sa_struct(void);
void *init_data_structs(start_args *s);
void init_stacks(start_args *s);
void init_mutex(start_args *s);
void init_sem(start_args *s);
void check_start_args(int argc, char *argv[], start_args *s);
void safe_threads(pthread_t thread[], start_args *s);
void join_threads(pthread_t thread[], start_args *s);
bool work_available(Stack *s, pthread_mutex_t *lock, char **str);
bool active_worker(start_args *s, pthread_mutex_t *mutex, int count);
void mark_active_work(int *active_workers, pthread_mutex_t *mutex, int value);
void do_work(start_args *s, char *str, int count);
void *work_func(void *arg);
void print_results(start_args *s);
void error_handling(start_args *s, char *str, int mode);
void *safe_calloc(int amount, size_t size);
void realloc_buff(char ***buffer, start_args *s);
void safe_exit(start_args *s);

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
    s->thread_error = 0;

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
        d[i]->active_work = 0;
    }

    return d;
}

/**
 * @brief Initializes the work_order stacks
 *
 * @param s start_args struct
 */
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
 * @brief Initialize mutex locks
 *
 * @param s start_args struct
 */
void init_mutex(start_args *s)
{
    s->locks = safe_calloc(5, sizeof(pthread_mutex_t *));

    for (int i = 0; i < 5; i++)
    {
        s->locks[i] = safe_calloc(1, sizeof(pthread_mutex_t));
        pthread_mutex_init(s->locks[i], NULL);
    }
}

/**
 * @brief Initialize semaphore
 *
 * @param s start_args struct
 */
void init_sem(start_args *s)
{
    s->dir_cond = safe_calloc(s->c_files, sizeof(sem_t));

    for (int i = 0; i < s->c_files; i++)
    {
        s->dir_cond[i] = safe_calloc(1, sizeof(sem_t));
        sem_init(s->dir_cond[i], 0, 0);
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

/**
 * @brief start threads safely
 *
 * @param thread array with all pthread_t
 * @param s start_args struct
 */
void safe_threads(pthread_t thread[], start_args *s)
{
    /* Start work threads */
    for (int i = 0; i < s->n_threads; i++)
    {
        if (pthread_create(&thread[i], NULL, work_func, s) != 0)
        {
            perror("pthread_create");
        }
    }
}

/**
 * @brief join all threads safely
 *
 * @param thread array with all pthread_t
 * @param s start_args struct
 */
void join_threads(pthread_t thread[], start_args *s)
{
    for (int i = 0; i < s->n_threads; i++)
    {
        if (pthread_join(thread[i], NULL) != 0)
        {
            perror("pthread_join");
        }
    }
}

/**
 * @brief checks for work in stack using mutex lock to avoid thread racing
 *
 * @param s start_args struct
 * @return true if work available otherwise false
 */
bool work_available(Stack *s, pthread_mutex_t *mutex, char **str)
{
    pthread_mutex_lock(mutex);
    if (!stack_is_empty(s))
    {
        *str = stack_pop(s);
        pthread_mutex_unlock(mutex);
        return true;
    }
    pthread_mutex_unlock(mutex);
    return false;
}

/**
 * @brief checks if any thread is currently working in a directory
 *
 * @param s start_args struct
 * @param count integer specifying which given file/dir is currently being worked on
 * @return true if there are active workers otherwise false
 */
bool active_worker(start_args *s, pthread_mutex_t *mutex, int count)
{
    pthread_mutex_lock(mutex);
    bool cond = false;
    if (s->data[count]->active_work != 0)
    {
        cond = true;
    }
    pthread_mutex_unlock(mutex);
    return cond;
}

/**
 * @brief increase or decrease amount of active workers for a directory
 *
 * @param active_workers pointer to integer with amount of active workers
 * @param mutex mutex lock
 * @param value marks if increase or decrease, -1 to decrease or 1 to increase
 */
void mark_active_work(int *active_workers, pthread_mutex_t *mutex, int value)
{
    pthread_mutex_lock(mutex);
    if (value > 0)
    {
        active_workers++;
    }
    else
    {
        active_workers--;
    }
    pthread_mutex_unlock(mutex);
}

/**
 * @brief handles all work collecting data from files and directories
 *
 * @param s start_args struct
 * @param str name of file/dir to execute work on
 * @param count integer specifying which given file/dir is currently being worked on
 */
void do_work(start_args *s, char *str, int count)
{
    /* Local variables declaration */
    struct dirent *ent_dir;
    DIR *dir;
    struct stat file_stat;
    long int local_total = 0;
    char *pathbuf;

    /* Mark that there's a thread working on the directory */
    mark_active_work(&s->data[count]->active_work, s->locks[2], 1);

    /* Get data of given file/dir and handle error */
    if (lstat(str, &file_stat) < 0)
    {
        error_handling(s, str, 0);
        return;
    }

    /*
     * If directory, opendir for readdir usage
     * If regular file, add block size to correct data struct
     */
    if (S_ISDIR(file_stat.st_mode))
    {
        /* Only add block size if given directory to avoid double addition if sub dir */
        pthread_mutex_lock(s->locks[3]);
        for (int i = 0; i < s->c_files; i++)
        {
            if (!strcmp(str, s->files[i]))
            {
                local_total += file_stat.st_blocks;
            }
        }
        pthread_mutex_unlock(s->locks[3]);

        if ((dir = opendir(str)) == NULL)
        {
            error_handling(s, str, 1);
            return;
        }
    }
    else if (S_ISREG(file_stat.st_mode))
    {
        local_total += file_stat.st_blocks;
        pthread_mutex_lock(s->locks[1]);
        s->data[count]->dir_total += local_total;
        pthread_mutex_unlock(s->locks[1]);

        mark_active_work(&s->data[count]->active_work, s->locks[2], -1);
        return;
    }

    /* readdir until it returns NULL at end of directory */
    while ((ent_dir = readdir(dir)) != NULL)
    {
        /* Skip links to parent dir */
        if (!strcmp(ent_dir->d_name, ".") || !strcmp(ent_dir->d_name, ".."))
        {
            continue;
        }

        /* Save correct path in pathbuf for new file/dir */
        pathbuf = safe_calloc(PATH_MAX, sizeof(char));
        snprintf(pathbuf, PATH_MAX, "%s/%s", str, ent_dir->d_name);

        if (lstat(pathbuf, &file_stat) < 0)
        {
            error_handling(s, str, 0);
        }

        /* If directory, add to work_order stack */
        if (S_ISDIR(file_stat.st_mode))
        {
            pthread_mutex_lock(s->locks[0]);
            stack_push(s->work_order[count], pathbuf);
            pthread_mutex_unlock(s->locks[0]);
            sem_post(s->dir_cond[count]); /* Notify waiting threads there's work */
        }

        /* Add block size to local_total and free pathbuf */
        local_total += file_stat.st_blocks;
        free(pathbuf);
    }

    /* Add total from read dir to correct data struct */
    pthread_mutex_lock(s->locks[1]);
    s->data[count]->dir_total += local_total;
    pthread_mutex_unlock(s->locks[1]);

    /* closedir, mark that work is finished */
    closedir(dir);
    mark_active_work(&s->data[count]->active_work, s->locks[2], -1);
}

/**
 * @brief Function used when starting threads
 *
 * @param arg
 * @return void*
 */
void *work_func(void *arg)
{
    start_args *s = (start_args *)arg;
    char *current;

    /* Loop through work_order stacks for all files and execute work */
    for (int i = 0; i < s->c_files; i++)
    {
        while (work_available(s->work_order[i], s->locks[0], &current))
        {
            do_work(s, current, i);
            free(current); /* Free memory of used string */

            if (active_worker(s, s->locks[2], i))
            {
                sem_wait(s->dir_cond[i]);
            }
        }
    }
    return 0;
}

/**
 * @brief Prints the results
 *
 * @param s start_args struct
 */
void print_results(start_args *s)
{
    for (int i = 0; i < s->c_files; i++)
    {
        printf("%ld     %s\n", s->data[i]->dir_total, s->files[i]);
    }
}

/**
 * @brief error handing in threads
 *
 * @param s start_args struct
 * @param str name of current dir/file
 * @param mode 0/1, used to specify if called after opendir
 */
void error_handling(start_args *s, char *str, int mode)
{
    if (mode == 1)
    {
        if (errno == EACCES)
        {
            /* Modified error message on EACCES that includes dir name */
            fprintf(stderr, "mdu: cannot read directory '%s': Permission denied\n", str);
        }
        else
        {
            perror"opendir");
        }
    }
    else
    {
        perror("error");
    }

    pthread_mutex_lock(s->locks[4]);
    s->thread_error = errno;
    pthread_mutex_unlock(s->locks[4]);
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
        perror("calloc");
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
        perror("realloc");
        exit(errno);
    }
    *buffer = temp;

    for (int i = 0; i < s->n_files; i++)
    {
        char *temp2;
        temp2 = realloc(*buffer[i], sizeof(char) * MAX_LINE);
        if (temp2 == NULL)
        {
            perror("realloc");
            exit(errno);
        }
        else
        {
            *buffer[i] = temp2;
        }
    }
}

/**
 * @brief Free allocated memory and destroy mutex locks before program exit
 *
 * @param s struct containing start arguments
 * @param locks[] mutex locks to be destroyed
 */
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