#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include "thread_funcs.h"
#include "mdu.h"

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