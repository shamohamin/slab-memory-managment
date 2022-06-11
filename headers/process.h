#ifndef __PROC_H__
#define __PROC_H__

#include "slab.h"

#define MAXIMUM_TRY_OF_GETTING_THE_MEMORY 2

typedef struct _KernelProc
{
    unsigned int id;
    // the first index is used for slab number and second index is used for CacheName
    int **indexes_of_occupied_pages; 
    char **request_name;
    int index_of_the_arrays;
} KernelProc;

typedef struct _ThreadArgs
{
    Cache **caches;
    KernelProc *proc;
    pthread_mutex_t *caches_lock;
} ThreadArgs;

void process_execuation(ThreadArgs *args);

int allocation(Cache *cache, KernelProc *proc, unsigned int size_of_object);
int allocation_process(ThreadArgs *args);

int deallocation(Cache *cache, KernelProc *proc, unsigned int index_of_deallocation);
int deallocation_process(ThreadArgs *args);

CacheNames request_OBJECT();

#endif