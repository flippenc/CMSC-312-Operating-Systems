#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/shm.h>

#define memsize 30

key_t intkey = 6391; key_t strkey = 3268;

void child_fun(int i)
{
	int intmem;
	int strmem;
	char *intmempointer;
	char *strmempointer;

	//i values: 0 = A, 1 = B, 3 = C

	 if (i == 0)
        {

                if ((intmem = shmget(intkey, memsize, IPC_CREAT | 0666)) < 0)
                {
                        perror("shmget");
                        exit(1);
                }

                if ((strmem = shmget(strkey, memsize, IPC_CREAT | 0666)) < 0)
                {
                        perror("shmget");
                        exit(1);
                }


                if ((intmempointer = shmat(intmem, NULL, 0)) == (char *) -1)
                {
                        perror("shmat");
                        exit(1);
                }

                if ((strmempointer = shmat(strmem, NULL, 0)) == (char *) -1)
                {
                        perror("shmat");
                        exit(1);
                }

                *intmempointer = '1';
                while (*intmempointer != '3')
                {
                        sleep(1);
                }

                printf("GoodBye\n");
                exit(0);
        }

	//This is the setup portion for B and C
	if (i == 1 || i == 2)
	{
		//wait 1 second for A to create the memory
		sleep(1);
		if ((intmem = shmget(intkey, memsize, 0666)) < 0)
        	{
                	perror("shmget");
                	exit(1);
        	}	

	        if ((strmem = shmget(strkey, memsize, 0666)) < 0)
        	{
        		perror("shmget");
        	        exit(1);
        	}

        	if ((intmempointer = shmat(intmem, NULL, 0)) == (char *) -1)
       		{
   			perror("shmat");
                	exit(1);
        	}

		if ((strmempointer = shmat(strmem, NULL, 0)) == (char *) -1)
        	{
        		perror("shmat");
        	        exit(1);
	        }

	}

	//This is the B process
	if (i == 1)
	{
		while (*intmempointer != '1')
		{
			sleep(1);
		}
		
		char *s;
		char *i;
		s = strmempointer;

		char str[6] = "shared";
	        int z;
       		for (z = 0; z<=5; z++)
       	 	{
                	*s++ = str[z];
        	}

		
		for (s = strmempointer; *s != (char) NULL; s++)
                {
                        putchar(*s);
                }
                putchar('\n');


		*intmempointer = '2';
		exit(0);
	}

	//This is the C process
	else if (i == 2)
	{
		while (*intmempointer != '2')
		{	
			sleep(1);
		}

		char *s;
		char *i;

		s = strmempointer;

		char str[6] = "memory";
        	int z;
        	for (z=0; z<=5; z++)
        	{
                	*s++ = str[z];
        	}
		
		for (s = strmempointer; *s != (char) NULL; s++)
		{
			putchar(*s);
		}
		putchar('\n');
		*intmempointer = '3';
		exit(0);
	}
}

int main ()
{
	int i;
	int pid;
	for (i = 0; i<3; i++)
	{
		pid = fork();
		if ( pid < 0 )
        	{
                	fprintf(stderr,"fork failed at %d\n",i);
                	exit(1);
        	}
        	else if ( pid > 0 )
        	{
			//The parent doesn't need to do anything
			//besides creating the children processes
                	continue;
        	}
        	else
        	{
			child_fun(i);
			return 0;
		}
	}
	//loop to wait for the children to exit, prints if any errors occured
	for (i = 0; i<3; i++)
	{
		int exitStatus;
		wait(&exitStatus);
		if (exitStatus > 0)
		{
			printf("Child exited with %d\n",WEXITSTATUS(exitStatus));
		}
	}
	return 0;
}
