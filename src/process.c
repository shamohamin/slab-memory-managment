
#include "../headers/process.h"
#include "math.h"

CacheNames request_OBJECT()
{
    int random_number = (rand() % 3) + 1;
    switch (random_number)
    {
    case 1:
        return PROC_DESC;
    case 2:
        return SYNC_TOOLS;
    default:
        return FILES;
    };
}

int allocation(Cache *cache, KernelProc *proc, unsigned int size_of_object)
{
    int waiting = 1;
    int num_of_tries = 0;
    int return_val = 0;

    while (num_of_tries < MAXIMUM_TRY_OF_GETTING_THE_MEMORY)
    {
        int index_of_the_slab = -1;

        pthread_mutex_lock(&cache->cache_lock);
        for (int i = 0; i < cache->slabs_count; i++)
        {
            if (((cache->slabs[i])->free_space) > size_of_object)
            {
                waiting = 0;
                index_of_the_slab = i;
                break;
            }
        }

        if (waiting == 1)
        { // there was not the enough pages for the requested object size.
            // we free the lock for the other threads in the other cases
            // the deadlock might occur.
            num_of_tries++;
            printf("[INFO] Process number {%d} went to the watting mode\n", proc->id);
            fflush(stdout);
            pthread_mutex_unlock(&cache->cache_lock);
            continue;
        }

        return_val = 1;
        int num_of_pages_required_to_cover = (int)(floor(size_of_object / MAX_PAGE_SIZE)) + 1;
        int temp = num_of_pages_required_to_cover;

        if (num_of_pages_required_to_cover == 0)
            break;
        else
            proc->indexes_of_occupied_pages[proc->index_of_the_arrays] = (int *)malloc(sizeof(int) * (num_of_pages_required_to_cover + 4));

        // printf("proc {%d} has requested for %dkb num of pages %d\n", proc->id, size_of_object, num_of_pages_required_to_cover);
        // fflush(stdout);
        int pages_index_to_be_filled = 3;
        proc->indexes_of_occupied_pages[proc->index_of_the_arrays][0] = index_of_the_slab;
        proc->indexes_of_occupied_pages[proc->index_of_the_arrays][1] = cache->cacheName;
        proc->indexes_of_occupied_pages[proc->index_of_the_arrays][2] = size_of_object;

        int occupied_size = 0;
        for (int i = 0; i < (cache->slabs[index_of_the_slab])->pages_count; i++)
        {
            if (num_of_pages_required_to_cover == 0)
                break;

            if ((cache->slabs[index_of_the_slab])->pages[i] == FREE)
            {
                (cache->slabs[index_of_the_slab])->pages[i] = USED;
                occupied_size = occupied_size + MAX_PAGE_SIZE;
                num_of_pages_required_to_cover--;
                proc->indexes_of_occupied_pages[proc->index_of_the_arrays][pages_index_to_be_filled] = i;
                pages_index_to_be_filled++;
            }
        }
        // indicates the end of array.
        proc->indexes_of_occupied_pages[proc->index_of_the_arrays][pages_index_to_be_filled] = -1;

        (cache->slabs[index_of_the_slab])->used_space += occupied_size;
        (cache->slabs[index_of_the_slab])->free_space = (cache->slabs[index_of_the_slab])->size - (cache->slabs[index_of_the_slab])->used_space;
        (cache->slabs[index_of_the_slab])->internal_fragmentation += (temp * MAX_PAGE_SIZE) - size_of_object;

        proc->index_of_the_arrays = proc->index_of_the_arrays + 1;
        pthread_mutex_unlock(&cache->cache_lock);
        break;
    }

    return return_val;
}

int allocation_process(ThreadArgs *args)
{
    CacheNames cache_name = request_OBJECT();
    Cache *temp_cache = NULL;

    pthread_mutex_lock(args->caches_lock);

    for (int i = 0; i < CACHE_DEFAULT_SIZE; i++)
        if ((args->caches[i])->cacheName == cache_name)
            temp_cache = args->caches[i];

    pthread_mutex_unlock(args->caches_lock);

    if (temp_cache == NULL)
        return 0;

    if (allocation(temp_cache, args->proc, (rand() % 10) + 1) == 1)
        return 1;

    return 0;
}

int deallocation(Cache *cache, KernelProc *proc, unsigned int index_of_deallocation)
{
    int return_temp = 0;
    pthread_mutex_lock(&cache->cache_lock);
    int slab_number = proc->indexes_of_occupied_pages[index_of_deallocation][0];
    int size_of_requested_object = proc->indexes_of_occupied_pages[index_of_deallocation][2];

    int pages_count = 0;
    for (int i = 3; proc->indexes_of_occupied_pages[index_of_deallocation][i] != -1; i++)
    {
        cache->slabs[slab_number]->free_space += MAX_PAGE_SIZE;
        cache->slabs[slab_number]->used_space -= MAX_PAGE_SIZE;
        pages_count++;
        cache->slabs[slab_number]->pages[proc->indexes_of_occupied_pages[index_of_deallocation][i]] = FREE;
    }
    
    cache->slabs[slab_number]->internal_fragmentation -= abs(size_of_requested_object - (MAX_PAGE_SIZE * pages_count));

    free(proc->indexes_of_occupied_pages[index_of_deallocation]);
    // shift the array.
    for (int i = index_of_deallocation + 1; i < proc->index_of_the_arrays; i++)
        proc->indexes_of_occupied_pages[i - 1] = proc->indexes_of_occupied_pages[i];
    proc->index_of_the_arrays -= 1;
    pthread_mutex_unlock(&cache->cache_lock);

    return return_temp;
}

int deallocation_process(ThreadArgs *args)
{
    Cache *temp_cache = NULL;

    pthread_mutex_lock(args->caches_lock);
    int requests_count = args->proc->index_of_the_arrays;
    int index_of_deallocation = rand() % requests_count;
    int slab_index = args->proc->indexes_of_occupied_pages[index_of_deallocation][0]; // index of slab
    int cache_name = args->proc->indexes_of_occupied_pages[index_of_deallocation][1]; // index of cache

    for (int i = 0; i < CACHE_DEFAULT_SIZE; i++)
        if ((args->caches[i])->cacheName == cache_name)
            temp_cache = args->caches[i];

    pthread_mutex_unlock(args->caches_lock);

    if (temp_cache == NULL)
        return 0;

    if (deallocation(temp_cache, args->proc, index_of_deallocation))
        return 1;

    return 0;
}

void process_execuation(ThreadArgs *args)
{
    int num_of_request = 0;

    while (num_of_request < MAX_NUM_OF_REQUEST)
    {
        if (rand() % 2 == 0)
        { // allocation
            if (allocation_process(args))
                num_of_request++;
            else
                continue;
        }
        else
        { // deallocation
            if (!((args->proc)->index_of_the_arrays))
                continue; // there was no allocated page for this process.
            
            deallocation_process(args);
        }

        // sleep:
#ifdef _WIN32
        Sleep(1000 * 2); // 2 secs
#else
        usleep(2 * 1000 * 1000); // 2 secs
#endif
    }
}