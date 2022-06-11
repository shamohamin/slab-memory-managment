
#ifndef __SLAB_H__
#define __SLAB_H__

#include "stdio.h"
#include <pthread.h>
#include <stdlib.h>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#endif

#define MAX_CACHE_SIZE 2048
#define MAX_SLAB_SIZE 1024
#define MAX_PAGE_SIZE 4

#define NUM_OF_WORKERS 10
#define MAX_NUM_OF_REQUEST 4
#define CACHE_DEFAULT_SIZE 3

typedef enum _PageStatus
{
    FREE = 0,
    USED,
} PageStatus;

typedef enum _CacheNames
{
    PROC_DESC = 0,
    SYNC_TOOLS,
    FILES
} CacheNames;

typedef struct _Slab
{
    unsigned int size;
    unsigned int used_space;
    unsigned int free_space;
    unsigned int internal_fragmentation;
    unsigned int pages_count;
    PageStatus *pages;
} Slab;

typedef struct _Cache
{
    CacheNames cacheName;
    unsigned int size;
    unsigned int slabs_count;
    Slab **slabs;
    pthread_mutex_t cache_lock;
} Cache;

typedef struct _CacheReporter
{
    Cache **caches;
} CacheReporter;

Slab *build_slab(unsigned int num_of_pages);
Cache **build_caches(unsigned int num_of_slabs, unsigned int num_of_pages);
void print_cache_summary(Cache **caches);

#endif