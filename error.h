/**
 * @author  Vincent Johansson
 * @since   2021-01-19
 *
 */

#ifndef ERROR_H
#define ERROR_H

#include "mdu.h"

/**
 * @brief error handing in threads
 *
 * @param s start_args struct
 * @param str name of current dir/file
 * @param mode 0/1, used to specify if called after opendir
 */
void error_handling(start_args *s, char *str, int mode);

/**
 * @brief Safe usage of calloc function
 *
 * @param size      size of memory to be allocated
 * @return void*    pointer to allocated memory
 */
void *safe_calloc(int amount, size_t size);

/**
 * @brief Reallocate **char buffer.
 *
 * @param buffer  	Pointer to **char buffer.
 * @param s			start_args struct.
 */
void realloc_buff(char ***buffer, start_args *s);

#endif /* ERROR_H */