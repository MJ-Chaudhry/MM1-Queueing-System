/**
 * Main header file for the system. 
 * 
 * This defines the structures and function prototypes used.
 */
#ifndef _SYSTEM_H
#define _SYSTEM_H

#include <stdlib.h>

#define SYSTEM_BUFFER_LEN (512)

typedef struct System {
    size_t no_customers;
    float *iats;
    float *arrival_times;
    float *service_times;

    float *service_start_times;
    float *service_end_times;

    float *wait_times;
    float *times_in_system;

    int *no_system;
    int *no_queue;
    float *idle_times;
} System;

/**
 * Initialize the system with `n` customers.
 *
 * This will also create an `input.csv` file for the IATs and service times generated.
 */
System *InitializeSystem(size_t n);

/**
 * Initialize the system from a file instead
 */
System *InitializeSystemFromFile(const char *input_file);

/**
 * Free any allocated memory for the system.
 */
void DestroySystem(System *);

#endif  // _SYSTEM_H