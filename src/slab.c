
#include "../headers/slab.h"
#include "../headers/errors.h"
/**
 * @brief
 *
 * @param num_of_pages
 * @return Slab*
 */
Slab *build_slab(unsigned int num_of_pages)
{
    unsigned int temp_num_of_pages = num_of_pages;
    if (num_of_pages == -1)
        temp_num_of_pages = (unsigned int)(MAX_SLAB_SIZE / MAX_PAGE_SIZE);

    Slab *temp_slab = (Slab *)malloc(sizeof(Slab));
    if (!temp_slab)
        ERROR_HANDLER_AND_DIE("memory allocation for slab was not successful.");

    temp_slab->free_space = temp_num_of_pages * MAX_PAGE_SIZE;
    temp_slab->internal_fragmentation = 0;
    temp_slab->size = temp_num_of_pages * MAX_PAGE_SIZE;
    temp_slab->used_space = 0;
    temp_slab->pages_count = temp_num_of_pages;
    temp_slab->pages = (PageStatus *)malloc(sizeof(PageStatus) * temp_num_of_pages);
    if (!temp_slab->pages)
        ERROR_HANDLER_AND_DIE("memory allocation for pages was not successful.")

    for(int i = 0; i < temp_num_of_pages; i++)
        (temp_slab->pages)[i] = FREE;

    return temp_slab;
}

Cache **build_caches(
    unsigned int num_of_slabs,
    unsigned int num_of_pages)
{
    unsigned int temp_num_of_caches = CACHE_DEFAULT_SIZE;
    unsigned int temp_num_of_slabs = num_of_slabs;

    if (num_of_slabs == -1)
        temp_num_of_slabs = (unsigned int)(MAX_CACHE_SIZE / MAX_SLAB_SIZE);

    CacheNames names[CACHE_DEFAULT_SIZE] = {PROC_DESC, SYNC_TOOLS, FILES};

    Cache **caches = (Cache **)malloc(sizeof(Cache *) * temp_num_of_caches);
    if (caches == NULL)
        ERROR_HANDLER_AND_DIE("memory allocation for caches wasnot successful.");

    for (int i = 0; i < temp_num_of_caches; i++)
    {
        Cache *temp_cache = (Cache *)malloc(sizeof(Cache));

        if (temp_cache == NULL)
            ERROR_HANDLER_AND_DIE(sprintf("memory allocation for cache number {%d} was not successful.", i));

        temp_cache->cacheName = names[i];
        temp_cache->size = temp_num_of_slabs * MAX_SLAB_SIZE; // calculate the size from slabs
        temp_cache->slabs = (Slab **)malloc(sizeof(Slab *) * temp_num_of_slabs);

        if (temp_cache->slabs == NULL)
            ERROR_HANDLER_AND_DIE("memory allocation for slabs wasnot successful.");

        for (int j = 0; j < temp_num_of_slabs; j++)
            temp_cache->slabs[j] = build_slab(num_of_pages);

        temp_cache->slabs_count = temp_num_of_slabs;

        if (pthread_mutex_init(&temp_cache->cache_lock, NULL) != 0)
            ERROR_HANDLER_AND_DIE("lock init incomplete.");

        caches[i] = temp_cache;
    }

    return caches;
}

void print_cache_summary(Cache **caches)
{
    for (int i = 0; i < CACHE_DEFAULT_SIZE; i++)
    {
        if (caches[i] == NULL)
            ERROR_HANDLER_AND_DIE(sprintf("cache {%d} has been not allocated", i));

        printf("NAME OF CACHE %d\n", caches[i]->cacheName);
        printf("SIZE OF CACHE %d\n", caches[i]->size);
        printf("SLABS OF CACHE %d\n", caches[i]->slabs_count);
        printf("*************°°***********°°*******°°*****\n");
    }
}