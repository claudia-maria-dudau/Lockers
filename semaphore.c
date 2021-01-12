#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include "synclibcm.h"

#define NTHRS 5

struct barrier
{
    mtxcm mtx;       //mutex
    semcm sem;       //semcmafor
    int nr;          //nr de thread-uri ce au ajuns la bariera
} bar;

//constructor
int init(int n)
{
    //creare mutex
    mtxcm_init(&bar.mtx);

    //creare semcmafor
    semcm_init(&bar.sem, 0);

    bar.nr = 0;
}

//destructor
void des()
{
    //distrugere mutex
    mtxcm_destroy(&bar.mtx);

    //distrugere semcmafor
    semcm_destroy(&bar.sem);
}

void barrier_point()
{
    //incrementare nr de thread-uri ce au ajuns la bariera
    mtxcm_lock(&bar.mtx);
    bar.nr ++;

    if (bar.nr != NTHRS) //daca nu ajuns toate thread-urile
    {
        mtxcm_unlock(&bar.mtx);
        semcm_wait(&bar.sem); //asteapta la abriera
    }
    else
    {
        mtxcm_unlock(&bar.mtx);
        for(int i = 0; i < NTHRS - 1; i++) //dam drumul la thread-uri
            semcm_post(&bar.sem);
    }
}

void * tfun(void *v)
{
    int *tid = (int *)v;

    printf("%d reached the barrier\n", *tid);
    barrier_point();
    printf("%d passed the barrier\n", *tid);

    return NULL;
}

int main()
{
    //creare bariera
    init(NTHRS);

    printf("NTHRS=%d\n", NTHRS);

    //creare thread-uri
    pthread_t *thrs = malloc(NTHRS * sizeof(pthread_t));
    if (thrs == NULL){
        perror(NULL);
        return errno;
    }

    int ids[NTHRS]; //vector cu id-urile thread-urilor
    for (int i = 0; i < NTHRS; i++)
    {
        ids[i] = i;
        pthread_create(&thrs[i], NULL, tfun, &ids[i]);
    }

    //reunire thread-uri la final
    for (int i = 0; i < NTHRS; i++)
        pthread_join(thrs[i], NULL);

    free(thrs);

    //distrugere bariera
    des();

    return 0;
}
