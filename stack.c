#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "stack.h"

Stack *stack_create(void)
{
    Stack *s;
    if ((s = calloc(1, sizeof(Stack))) == NULL)
    {
        perror("malloc");
        exit(errno);
    }
    s->top = NULL;
    s->size = 0;

    return s;
}

void stack_destroy(Stack *s)
{
    while (s->top != NULL)
    {
        stack_pop(s);
    }
    free(s);
}

void stack_push(Stack *s, char *str)
{
    /* Allocate memory for new data entry */
    stack_data *new_data;
    if ((new_data = malloc(sizeof(*new_data))) == NULL)
    {
        perror("malloc");
        exit(errno);
    }
    if ((new_data->data = malloc(sizeof(char) * strlen(str) + 1)) == NULL)
    {
        perror("malloc");
        exit(errno);
    }

    /* Copy string and redirect pointers */
    strcpy(new_data->data, str);
    new_data->prev = s->top;
    s->top = new_data;
    s->size++;
}

char *stack_pop(Stack *s)
{
    /* Allocate temp memory for string and copy it */
    char *tmp_string;
    if ((tmp_string = malloc(sizeof(char) * strlen(s->top->data) + 1)) == NULL)
    {
        perror("malloc");
        exit(errno);
    }
    strcpy(tmp_string, s->top->data);

    /* Redirect pointers */
    stack_data *tmp_data = s->top;
    s->top = s->top->prev;
    s->size--;

    /* Free memory of popped entry */
    free(tmp_data->data);
    free(tmp_data);

    return tmp_string;
}

bool stack_is_empty(const Stack *s)
{
    if (s->size == 0)
    {
        return true;
    }
    return false;
}