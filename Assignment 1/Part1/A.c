#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h> /* for exit */

#define memsize 30

int
main()
{
	int intmem;
	int strmem;
	key_t intkey = 5621; key_t strkey = 1623;
	//shmget allocates a shared memory segment
	//key is the key to access the shared memory, memsize is the size, and IPC_CREAT | 0666 specifies that the memory is being created and 0666 is usual access permission in linux, rwx in octal (owner,group,user)
	//shmget() returns the identifier of the shared memory segment associated with the value of the argument key
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

	char *intmempointer;	
	char *strmempointer;
	//shmat attaches the shared memory segment shmid to the address space of the calling process
	//if shmaddr is null, the system chooses a suitable unused address at which to attach the segment
	//The last parameter is flags
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
	return 0;
}
