#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

struct CSem
{
	int val;
	sem_t gate;
	sem_t mutex;
};

int min(int a, int b)
{
        if (a < b)
        {
                return a;
        }
        return b;
}

void counting_sem_init(struct CSem *sem, int K)
{
        sem->val = K;
        sem_init(&(sem->gate),1,min(1,K));
        sem_init(&(sem->mutex),1,1);
}


//P = sem_wait
//V = sem_post

void counting_sem_wait(struct CSem *sem)
{
	sem_wait(&(sem->gate));
	sem_wait(&(sem->mutex));
	sem->val--;
	if (sem->val > 0)
	{
		sem_post(&(sem->gate));
	}
	sem_post(&(sem->mutex));
}

void counting_sem_post(struct CSem *sem)
{
	sem_wait(&(sem->mutex));
	sem->val++;
	if (sem->val == 1)
	{
		sem_post(&(sem->gate));
	}
	sem_post(&(sem->mutex));
}

int counting_sem_destroy(struct CSem *sem)
{
	sem_destroy(&(sem->gate));
	sem_destroy(&(sem->mutex));
}
