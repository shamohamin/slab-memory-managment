#include "headers/slab.h"

int main()
{
    Cache** caches = build_caches(-1, -1);

    for(int i = 0; i < CACHE_DEFAULT_SIZE; i++)
    {
        printf("NAME OF CACHE %d\n", caches[i]->cacheName);
        printf("SIZE OF CACHE %d\n", caches[i]->size);
        printf("SLABS OF CACHE %d\n", caches[i]->slabs_count);
    }

    for(int i = 0; i < CACHE_DEFAULT_SIZE; i++)
    {
        free(caches[i]->slabs);
        free(caches[i]);
    }
}