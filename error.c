#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "error.h"

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
            perror("opendir");
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