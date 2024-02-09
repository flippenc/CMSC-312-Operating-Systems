#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h> 

#define memsize 30

int main()
{
	int intmem;
	int strmem;
	key_t intkey = 5621; key_t strkey = 1623;

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

	char *intmempointer;
	char *strmempointer;

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

	//Waiting for B to finish
	while(*intmempointer != '2')
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

	return 0;
}
