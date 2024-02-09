#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>

//Csem stands for counting semaphores, bsem stands for binary semaphores
struct CSem
{
	int val;
	//These sem_t's are binary semaphores
	//initialize gate and mutex as binary semaphores with values min(1,val)
	//and 1
	//sem_init() initializes the unanmed semaphore at the address pointed to by sem, the value argument specifies the initial value for the semaphore
	//sem_init( sem_t *sem, int pshared, unsigned int value)
	//if phshared is 0, the semaphore is shared between the threads of the process and should be located at some address that is visible to all threads - global variable or malloced variable
	sem_t gate;// = min(1,val);
	sem_t mutex;// = 1;
};

void counting_sem_init(struct CSem *sem, int K)
{
	sem->val = K;
	sem_init(&(sem->gate),0,min(1,K));
	sem_init(&(sem->mutex),0,1);
}

//THE VARIABLES IN CSEM MUST BE GLOBAL

//the 1: P(wait) and a1: lines in the implementations are just labelling the lines

//P = sem_wait
//V = sem_signal

//in the pdf, called Pc(cs)
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

//in the pdf, called Vc(cs), instead of signal, this will be used
/*oid counting_sem_signal(struct CSem *sem)
{
	printf("In the signal function\n");
	sem_wait(&(sem->mutex));
	sem->val = sem->val + 1;
	if (sem->val == 1)
	{
		signal(&(sem->gate));
	}
	signal(&(sem->mutex));
}*/

int min(int a, int b)
{
	if (a < b)
	{
		return a;
	}
	return b;
}

void counting_sem_post(struct CSem *sem)
{
	printf("In the post function\n");
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
	printf("In the destroy function\n");
	sem_destroy(&(sem->gate));
	sem_destroy(&(sem->mutex));
	sem->val = 0;
}
