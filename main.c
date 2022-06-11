#include "headers/errors.h"
#include "headers/process.h"
#include "time.h"

int done = 0;

void *thread_initialization(void *args)
{
    ThreadArgs *get_args = (ThreadArgs *)args;
    process_execuation(get_args);

    if (get_args->proc != NULL)
        free(get_args->proc);
}

void *reporter_thread(void *args)
{
    CacheReporter *actual_args = args;

    while (!done)
    {
#ifdef _WIN32
        Sleep(2 * 1000);
#else
        usleep(2 * 1000 * 1000);
#endif

        for (int i = 0; i < CACHE_DEFAULT_SIZE; i++)
        {
            pthread_mutex_lock(&((actual_args->caches[i])->cache_lock));
            printf("cache number {%d} summary is like this -> \n", i + 1);
            printf("\t total size = %dkb \n", actual_args->caches[i]->size);
            printf("\t number of slabs = %d \n", actual_args->caches[i]->slabs_count);
            fflush(stdout);
            for (int j = 0; j < actual_args->caches[i]->slabs_count; j++)
            {
                printf("\t\t slab number {%d} \n", j + 1);
                printf("\t\t\t total number of pages => %d\n", actual_args->caches[i]->slabs[j]->pages_count);
                printf("\t\t\t total size of slab => %dkb \n", actual_args->caches[i]->slabs[j]->size);
                printf("\t\t\t free space of slab => %dkb \n", actual_args->caches[i]->slabs[j]->free_space);
                printf("\t\t\t used space of slab => %dkb \n", actual_args->caches[i]->slabs[j]->used_space);
                printf("\t\t\t internal fragment space of slab => %dkb\n", actual_args->caches[i]->slabs[j]->internal_fragmentation);
                fflush(stdout);
            }
            printf("*****************\n");
            fflush(stdout);
            pthread_mutex_unlock(&((actual_args->caches[i])->cache_lock));
        }
    }
}

/**
 * @author Mohammadamin shafiee
 *
 * @brief the entry point of the slab memory managment.
 *
 * @return int
 */

int main()
{
    pthread_t threads[NUM_OF_WORKERS + 1];

    Cache **caches = build_caches(-1, -1);
    pthread_mutex_t cache_search_lock;

    if (pthread_mutex_init(&cache_search_lock, NULL) != 0)
        ERROR_HANDLER_AND_DIE("cache_search_lock init incomplete.");

    for (int i = 0; i < NUM_OF_WORKERS; i++)
    {
        ThreadArgs *arg = (ThreadArgs *)malloc(sizeof(*arg));
        arg->caches = caches;
        arg->caches_lock = &cache_search_lock;

        KernelProc *proc = (KernelProc *)malloc(sizeof(KernelProc));
        proc->id = i + 1;
        arg->proc = proc;
        (arg->proc)->indexes_of_occupied_pages = (int **)malloc(sizeof(int *) * MAX_NUM_OF_REQUEST);
        (arg->proc)->request_name = (char **)malloc(sizeof(char *) * MAX_NUM_OF_REQUEST);
        (arg->proc)->index_of_the_arrays = 0;

        if (pthread_create(&threads[i], NULL, thread_initialization, arg))
            ERROR_HANDLER_AND_DIE(sprintf("thread {%d} cannot be initialized.", i));
    }

    CacheReporter *reporter = (CacheReporter *)malloc(sizeof(CacheReporter));
    reporter->caches = caches;
    if (pthread_create(&threads[NUM_OF_WORKERS], NULL, reporter_thread, reporter))
        ERROR_HANDLER_AND_DIE("thread reporter cannot be initialized.");

    for (int i = 0; i < NUM_OF_WORKERS; i++)
        pthread_join(threads[i], NULL);

    done = 1;

    pthread_join(threads[NUM_OF_WORKERS], NULL); // reporter thread.

    // clean the heap shit
    for (int i = 0; i < CACHE_DEFAULT_SIZE; i++)
    {
        free(caches[i]->slabs);
        free(caches[i]);
    }
    free(reporter);
}