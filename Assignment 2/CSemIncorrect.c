#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

struct CSem
{
	int val;
	sem_t wait;
	sem_t mutex;
};

void counting_sem_init(struct CSem *sem, int K)
{
	sem->val = K;
	sem_init(&(sem->wait),0,0);
	sem_init(&(sem->mutex),0,1);
}

void counting_sem_wait(struct CSem *sem)
{
	sem_wait(&(sem->mutex));
	sem->val--;
	if (sem->val < 0)
	{
		sem_post(&(sem->mutex));
		sem_wait(&(sem->wait));
	}
	else
	{
		sem_post(&(sem->mutex));
	}
}

void counting_sem_post(struct CSem *sem)
{
	sem_post(&(sem->mutex));
	sem->val++;
	if (sem->val <= 0)
	{
		sem_post(&(sem->wait));
	}
	sem_post(&(sem->mutex));
}

void counting_sem_destroy(struct CSem *sem)
{
	sem_destroy(&(sem->wait));
	sem_destroy(&(sem->mutex));
	sem->val = 0;
}
