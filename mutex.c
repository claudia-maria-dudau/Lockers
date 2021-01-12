#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>
#include "synclibcm.h"

#define MAX_RESOURCES 5
#define NTHRS 10
int available_resources = MAX_RESOURCES;
mtxcm mtx;

int decrease_count(int count)
{      
    mtxcm_lock(&mtx);

    if (available_resources < count)
    {
        mtxcm_unlock(&mtx);
        return -1;
    }

    else
    {
        available_resources -= count;
        printf("Got %d resoruces %d remaining\n", count, available_resources);
        mtxcm_unlock(&mtx);
    }
    
    return 0;
}

int increase_count(int count)
{   
    mtxcm_lock(&mtx);

    available_resources += count;
    printf("Released %d resoruces %d remaining\n", count, available_resources);

    mtxcm_unlock(&mtx);

    return 0;
}


void *func(void *arg)
{
    int nr = *(int*)arg;

    //se asteapta pana se poate prelua nr necesar de resurse
    while (decrease_count(nr) == -1)
        ;

    //executia thread-ului
    for (int i = 0; i < nr; i++)
        ;

    //eliberare resurse
    increase_count(nr);
    return 0;
}

int main()
{
    srand(time(NULL)); //sa dea nr diferite la fiecare compilare

    printf("MAX_RESOURCES=%d\n", MAX_RESOURCES);

    //creare mutex
    mtxcm_init(&mtx);

    //creare thread-uri
    pthread_t *thrs = malloc(NTHRS * sizeof(pthread_t));
    if (thrs == NULL){
        perror(NULL);
        return errno;
    }

    for (int i = 0; i < NTHRS; i++)
    {
        int nr = rand() % (MAX_RESOURCES) + 1;
        pthread_create(&thrs[i], NULL, func, &nr);
    }

    //reunire thread-uri la final
    for (int i = 0; i < NTHRS; i++)
        pthread_join(thrs[i], NULL);

    //eliberare memorie
    free(thrs);
    mtxcm_destroy(&mtx);

    return 0;
}