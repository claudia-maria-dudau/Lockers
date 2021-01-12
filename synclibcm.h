#include <stdatomic.h>

typedef enum {false, true} bool;

typedef struct
{
	atomic_flag available;
	bool destroyed;
} mtxcm;

typedef struct
{
	atomic_int value;
	bool destroyed;
} semcm;

typedef struct
{
	mtxcm mtx1; //readers
    mtxcm mtx2; //writers
	atomic_int counter;
	bool destroyed;
} rwlockcm;

void mtxcm_init(mtxcm *mtx)
{
	mtx->available = (atomic_flag) ATOMIC_FLAG_INIT;
	mtx->destroyed = false;
}

void mtxcm_destroy(mtxcm *mtx)
{
	mtx->destroyed = true;
}

void mtxcm_lock(mtxcm *mtx)
{
	if (mtx->destroyed == false)
		while(atomic_flag_test_and_set(&mtx->available) == 1);   //spin until the lock is acquired
}

void mtxcm_unlock(mtxcm *mtx)
{
	if (mtx->destroyed == false)
		atomic_flag_clear(&mtx->available);
}

void semcm_init (semcm *sem, int value)
{
	sem->value = value;
	sem->destroyed = false;
}

void semcm_wait(semcm *sem)
{	
	if (sem->destroyed == false)
	{
		while(atomic_load(&sem->value) <= 0) //atomically obtain the value stored in an atomic object
			; //wait for the semaphore to become available
		atomic_fetch_sub(&sem->value, 1); //returns the value held previously by sem_value
	}
}

void semcm_post(semcm *sem)
{	
	if (sem->destroyed == false)
    		atomic_fetch_add(&sem->value, 1);
}

void semcm_destroy (semcm *sem)
{
    	sem->destroyed = true;
}

void rwlockcm_init (rwlockcm *rw)
{
	mtxcm_init(&rw->mtx1);
	mtxcm_init(&rw->mtx2);
	rw->counter = 0;
	rw->destroyed = false;
}

void rwlockcm_readbegin (rwlockcm *rw)
{
	if (rw->destroyed == false)
	{
		mtxcm_lock(&rw->mtx1);	//lock mtx1
		atomic_fetch_add(&rw->counter, 1);
		if (atomic_load(&rw->counter) == 1)	//if there's at least one reader, lock mtx2
			mtxcm_lock(&rw->mtx2);
		mtxcm_unlock(&rw->mtx1);
	}
}

void rwlockcm_readend (rwlockcm *rw)
{
	if (rw->destroyed == false)
	{
		mtxcm_lock(&rw->mtx1);	//lock mtx1
		atomic_fetch_sub(&rw->counter, 1);
		if (atomic_load(&rw->counter) == 0)	//if mtx2 was locked in readbegin, unlock it
			mtxcm_unlock(&rw->mtx2);
		mtxcm_unlock(&rw->mtx1);
	}
}

void rwlockcm_writebegin (rwlockcm *rw)
{
	if (rw->destroyed == false)
		mtxcm_lock(&rw->mtx2);
}

void rwlockcm_writeend (rwlockcm *rw)
{
	if (rw->destroyed == false)
		mtxcm_unlock(&rw->mtx2);
}

void rwlockcm_destroy (rwlockcm *rw)
{
	mtxcm_destroy(&rw->mtx1);
	mtxcm_destroy(&rw->mtx2);
	rw->destroyed = true;
}