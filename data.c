#include <pthread.h>
#include "data.h"
#include "error.h"
#include "mdu.h"

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

void init_stacks(start_args *s)
{
    s->work_order = safe_calloc(s->c_files, sizeof(Stack **));
    for (int i = 0; i < s->c_files; i++)
    {
        s->work_order[i] = stack_create();
        stack_push(s->work_order[i], s->files[i]);
    }
}

void init_mutex(start_args *s)
{
    s->locks = safe_calloc(5, sizeof(pthread_mutex_t *));

    for (int i = 0; i < 5; i++)
    {
        s->locks[i] = safe_calloc(1, sizeof(pthread_mutex_t));
        pthread_mutex_init(s->locks[i], NULL);
    }
}

void init_sem(start_args *s)
{
    s->dir_cond = safe_calloc(s->c_files, sizeof(sem_t));

    for (int i = 0; i < s->c_files; i++)
    {
        s->dir_cond[i] = safe_calloc(1, sizeof(sem_t));
        sem_init(s->dir_cond[i], 0, 0);
    }
}