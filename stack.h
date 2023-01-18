/*
 * @author  Vincent Johansson
 * @since   2021-01-03
 *
 */

#ifndef STACK_H
#define STACK_H

#include <stdbool.h>
#include <errno.h>

/**
 * @defgroup module Stack
 *
 * @brief Stack.
 *
 * Stack implemented with the use of a dynamic array.
 *
 * @{
 */

typedef struct stack_data
{
    char *data;
    struct stack_data *prev;
} stack_data;

/**
 * @brief The type for the stack.
 *
 * Definition of the struct stack.
 *
 */
typedef struct stack
{
    struct stack_data *top;
    int size;
} Stack;

/**
 * @brief Creates an empty stack.
 *
 * Creates an empty stack.
 *
 * @return s Pointer to the empty stack.
 */
Stack *stack_create(void);

/**
 * @brief Destroys stack.
 *
 * Takes given stack *s and frees all allocated memory.
 *
 * @param *s Stack that is to be destroyed.
 */
void stack_destroy(Stack *s);

/**
 * @brief Insert a value into the stack.
 *
 * Takes given stack *s and adds given value.
 *
 * @param *s The stack where the value is to be inserted.
 * @param value The value that is to be added.
 */
void stack_push(Stack *s, char *str);

/**
 * @brief Removes a value from the stack.
 *
 * Takes given stack *s and removes the last added value.
 *
 * @param *s The stack where the value is to be removed from.
 * @return value The value that got removed from the stack.
 */
char *stack_pop(Stack *s);

/**
 * @brief Check if the stack is empty.
 *
 * Takes given stack *s and checks if it's empty.
 *
 * @param *s The stack we wish to check if it's empty.
 * @return true if empty, false if not.
 */
bool stack_is_empty(const Stack *s);

#endif /* STACK_H */
