
#ifndef __SLAB_H__
#define __SLAB_H__

#define MAX_CACHE_SIZE 2048 #kb
#define MAX_SLAB_SIZE 1024 #kb
#define MAX_PAGE_SIZE 4 #kb

typedef enum _PageStatus
{
    FREE = 0,
    USED,
} PageStatus;

typedef struct _Slab
{
    unsigned int size;
    unsigned int used_space;
    unsigned int free_space;
    unsigned int internal_fragmentation;
    PageStatus *pages;
} Slab;

typedef struct _Cache
{
    unsigned int size;
    unsigned int slabs_count;
    Slab *slabs;
} Cache;

export build_slab();

#endif