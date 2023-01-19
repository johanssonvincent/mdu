/**
 * @author  Vincent Johansson
 * @since   2021-01-19
 *
 */

#ifndef DATA_H
#define DATA_H

#include "mdu.h"

/**
 * @brief Initialize start_args struct
 *
 * @return void* pointer to allocated memory
 */
void *init_sa_struct(void);

/**
 * @brief Initialize data struct
 *
 * @param s start_args struct
 * @return void* pointer to allocated memory
 */
void *init_data_structs(start_args *s);

/**
 * @brief Initializes the work_order stacks
 *
 * @param s start_args struct
 */
void init_stacks(start_args *s);

/**
 * @brief Initialize mutex locks
 *
 * @param s start_args struct
 */
void init_mutex(start_args *s);

/**
 * @brief Initialize semaphore
 *
 * @param s start_args struct
 */
void init_sem(start_args *s);

#endif /* DATA_H */