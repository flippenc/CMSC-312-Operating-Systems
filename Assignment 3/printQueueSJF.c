#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <sys/mman.h>
#include <time.h>

#include <pthread.h>
#include <semaphore.h>
#include "CSemCorrect.c"

#define MAXJOBS 30
#define MAXSIZE 1000

//number of users and printers as set by the input parameters
int numUsers;
int numPrinters;

//ids for shared memory segments

//semaphore shared memory
int semShared = -1;

pthread_t *thread;

struct timespec timeStart, timeEnd;

//total number of jobs shared - used for consumer to know when to finish
int totalNumJobsShared = 0;

size_t mmapSize = 0;

//keys for shared memory segments
int sharedKeySem = 8776;
int sharedKeyNumJobs = 5806;

//semaphores for when the queue is full or empty, mutex is stored in the jobs
struct CSem full_sem;
struct CSem empty_sem;

//pointers for shared memory segments
struct CSem* sem_ptr;
void* queue_ptr;
int* total_num_jobs_ptr;

struct job
{
        int size;       //size of the job (1-100 bytes)
        int id;         //id of the process that created it
	sem_t mutex;	//mutex semaphore for access to this job
	struct job* next; //pointer to the next job in the queue
};

//there are 2 lists in shared memory
//job_head points to a list of "full" jobs - i.e. jobs that need to be consumed
//free_head points to a list of "empty" jobs - jobs that can be pointed to by the producer 
//in order to add them to the printer queue
static struct job **job_head;
static struct job **free_head;

void child_SIGINT_handler(int sig)
{
        //printf("Received SIGINT in child\n");
        exit(0);
}


void parent_SIGINT_handler(int sig)
{
	clock_gettime(CLOCK_REALTIME, &timeEnd);

        printf("SIGINT received, killing\n");
        if ( sem_ptr != NULL )
        {
                printf("Closing semaphore shared memory\n");
                shmdt(sem_ptr);
                shmctl(semShared, IPC_RMID, NULL);
        }

	if ( total_num_jobs_ptr != NULL)
	{
		printf("Closing total num jobs shared memory\n");
		shmdt(total_num_jobs_ptr);
		shmctl(totalNumJobsShared, IPC_RMID, NULL);
	}

	if (queue_ptr != NULL)
	{
		printf("Closing printer queue\n");
		munmap(queue_ptr, mmapSize);
	}

	counting_sem_destroy(&full_sem);
        counting_sem_destroy(&empty_sem);

	printf("Time elapsed is %f seconds\n", 
	(timeEnd.tv_sec - timeStart.tv_sec) + ((timeEnd.tv_nsec - timeStart.tv_nsec)/1E9));

        raise(SIGTERM);
}


void print_list(void)
{
	struct job *current = *job_head;
	while (current != NULL)
	{
		printf("Job from process %d with size %d->", current->id, current->size);
		current = current->next;
	}
	printf("\n");
}

int producer(int pid)
{
	struct sigaction new_action, old_action;
        new_action.sa_handler = child_SIGINT_handler;
        new_action.sa_flags = SA_NODEFER | SA_ONSTACK;
        sigaction(SIGINT, &new_action, &old_action);

	struct CSem *s;
	s = sem_ptr;

	struct CSem *full;
	full = s++;

	struct CSem *empty;
	empty = s;

	struct job *j;
	struct job *f;

	srand(time(NULL)+pid);
	int numJobs = rand()%MAXJOBS+1;
	*total_num_jobs_ptr = *total_num_jobs_ptr + numJobs;

	int i;
	for ( i = 0 ; i < numJobs; i++ )
	{
		double delay = (rand()%1000000);
		usleep(delay*2);
	
		int size = (rand()%900)+100;

		//wait for the printer queue to have room for a job
		counting_sem_wait(full);

		//we know free_head is never null because of the full sem

		j = *job_head;
		if ( j == NULL)
		{	
			f = *free_head;
			sem_wait(&(f->mutex));
			j = f;	
				
			f = f->next;
			
			j->size = size;
			j->id = pid;
                        j->next = NULL;
			
			sem_post(&(j->mutex));

			*free_head= f;
			*job_head = j;
			
			printf("Producer %d added %d to buffer\n", pid, size);
			counting_sem_post(empty);
		}
		else
		{
			//j is *job_head
			struct job * storeJobHead = *job_head;
			struct job * newEnd = *free_head;
			struct job * oldFreeHead = *free_head;
			sem_wait(&(oldFreeHead->mutex));
			newEnd->size = size;
			newEnd->id = pid;
			*free_head = (*free_head)->next;
		
			//add new front
			if (j->size > size)
			{
				sem_wait(&(j->mutex));
				
				newEnd->next = *job_head;
				
				sem_post(&(j->mutex));
				
				*job_head = newEnd;

				printf("Producer %d added %d to buffer\n", pid, size);

				counting_sem_post(empty);
			}
			else
			{
				while(j->next != NULL)
				{
					if ( j->next->size > size)
					{
						break;
					}
					else
					{
						j = j->next;
					}
				}
				if (j->next == NULL)
				{
					//Adding to the end of the list
					sem_wait(&(j->mutex));

					newEnd->next = NULL;
					
					j->next = newEnd;
					*job_head = storeJobHead;
					
					sem_post(&(j->mutex));

					printf("Producer %d added %d to buffer\n", pid, size);

					counting_sem_post(empty);
				}
				else //j->next != NULL, we are adding to the middle of the list
				{
					sem_wait(&(j->mutex));

					newEnd->next = j->next;
					
					j->next = newEnd;
					*job_head = storeJobHead;

					sem_post(&(j->mutex));

					printf("Producer %d added %d to buffer\n", pid, size);

					counting_sem_post(empty);
				}
			}
	                sem_post(&(oldFreeHead->mutex));
		}
	}
	printf("Closing producer\n");
	exit(0);
}

void *consumer(void *thread_n)
{
	int thread_numb = *(int *)thread_n;

	struct job *j;
	j = *job_head;

	struct CSem *s;
        s = sem_ptr;

        struct CSem *full;
        full = s++;

        struct CSem *empty;
        empty = s;

	int currentID = -1;
	int currentSize = -1;

	sleep(1);
	while(*total_num_jobs_ptr > 0)
	{
		//printf("Total number of jobs currently is %d\n", *total_num_jobs_ptr);
		counting_sem_wait(empty);

		struct job * oldJobHead = *job_head;
		struct job * oldFreeHead = *free_head;

		struct job * j = *job_head;
		if ( j != NULL)
		{
			sem_wait(&(oldJobHead->mutex));
		}

		struct job * f = *free_head;
		if ( f != NULL)
		{
			sem_wait(&(oldFreeHead->mutex));
		}

		if ( f == NULL)
		{
			f = j;
			j = j->next;

			currentID = f->id;
			currentSize = f->size;

			f->id = -1;
			f->size = -1;
			f->next = NULL;

			*job_head = j;

			//unlocking job_head
			sem_post(&(oldJobHead->mutex));
			*free_head = f;
			
		}
		else
		{
			struct job * storeJobHead = *job_head;
			struct job * newFront = *job_head;
			
			*job_head = (*job_head)->next;

			currentID = storeJobHead->id;
			currentSize = storeJobHead->size;
			
			newFront->id = -1;
			newFront->size = -1;
			newFront->next = *free_head;

			sem_post(&(oldJobHead->mutex));
			*free_head = newFront;
			sem_post(&(oldFreeHead->mutex));
		}

		*total_num_jobs_ptr = *total_num_jobs_ptr-1;
		usleep(currentSize*1000);
		printf("Consumer %d removed %d from user: %d\n", thread_numb, currentSize, currentID);
		counting_sem_post(full);
//		usleep(currentSize*1000);
	}
	raise(SIGINT);
}

int main(int argc, char **argv)
{

	if ( argc < 3 )
        {
                printf("usage: %s numUsers numPrinters\n",argv[0]);
                exit(1);
	}

        if ( argc >= 4 )
        {
                printf("Too many input values\n");
                exit(2);
        }

	struct sigaction new_action, old_action;
        new_action.sa_handler = parent_SIGINT_handler;
        new_action.sa_flags = SA_NODEFER | SA_ONSTACK;
        sigaction(SIGINT, &new_action, &old_action);

        numUsers = atoi(argv[1]);
        numPrinters = atoi(argv[2]);
        printf("%i users and %i printers\n",numUsers,numPrinters);

	if ((semShared = shmget(sharedKeySem, 2*sizeof(struct CSem), IPC_CREAT | 0666)) < 0)
        {
                perror("shmget");
                exit(1);
        }

	if ((sem_ptr = shmat(semShared, NULL, 0)) == (struct CSem*) -1)
        {
                perror("shmat");
                exit(1);
        }

	if ((totalNumJobsShared = shmget(sharedKeyNumJobs, sizeof(int), IPC_CREAT | 0666)) < 0)
	{
		perror("shmget");
		exit(1);
	}

	if ((total_num_jobs_ptr = shmat(totalNumJobsShared, NULL, 0)) == (int*) -1)
	{
		perror("shmat");
		exit(1);
	}

	*total_num_jobs_ptr = 0;

	counting_sem_init(&full_sem, MAXJOBS);
        counting_sem_init(&empty_sem, 0);
	
	printf("Initialized full and empty counting semaphores\n");

	struct CSem* s;
	s = sem_ptr;
	
	*s++ = full_sem;
	*s++ = empty_sem;

	mmapSize += sizeof(**job_head)*30;
	mmapSize += sizeof(job_head)+sizeof(free_head);	

	queue_ptr = mmap(NULL, mmapSize, PROT_READ | PROT_WRITE, MAP_SHARED | MAP_ANONYMOUS, 0, 0);
	if (queue_ptr == MAP_FAILED)
	{
		perror("Memory map failed");
		exit(2);
	}

	printf("Made shared memory segments\n");

	free_head = (struct job **) ((char *) queue_ptr);
        job_head = free_head+1;

        *free_head = (struct job *) (job_head+1);
	*job_head = NULL;

        int i;
        struct job *current;

        for (i = 0 , current = *free_head; i < MAXJOBS-1; i++, current++)
        {
                current->id = -1;
                current->size = -1;
                sem_init(&(current->mutex),1,1);
                current->next = current+1;
        }

	current->next = NULL;

	printf("Initialized printer queue\n");

	clock_gettime(CLOCK_REALTIME, &timeStart);
	//printf("Start time is %f\n",(timeStart.tv_sec - (timeStart.tv_nsec)/1E9));

	int pid;
	for ( i = 0; i < numUsers; i++)
	{
		pid = fork();
		if (pid < 0)
		{
			fprintf(stderr,"forked failed at %d\n",i);
			exit(1);
		}
		else if ( pid > 0)
		{
			continue;
		}
		else
		{
			printf("Entering producer with process %d\n",i);
			producer(i);
			return 0;
		}
	}
	
	printf("Made all of the users\n");

	thread = malloc(numPrinters*sizeof(pthread_t));
	int thread_numb[numPrinters];
	for ( i = 0; i <numPrinters; i++)
	{
		thread_numb[i] = i;
		printf("Made printer %d\n",i);
		pthread_create(thread+i, NULL, consumer, thread_numb + i);
	}
	
	printf("Made all of the printers\n");

	for ( i = 0; i < numPrinters; i++)
        {
                pthread_join(thread[i], NULL);
        }

	raise(SIGINT);
	
	return 0;
}

