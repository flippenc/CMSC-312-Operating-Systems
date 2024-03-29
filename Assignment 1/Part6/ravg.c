
#include "avg.h"	/* header file generated by rpcgen */ 
#include <stdlib.h>
#include <strings.h>

//Sorttype is ascending or descending
//1 for ascending, -1 for descending
int sortType = 0;

/* local routine client */
/* prototype can be whatever you want */
void averageprog_1( char* host, int argc, char *argv[] )
{
	CLIENT *clnt; /* client handle, rpc.h included in avg.h from rpcgen */
	int i;
	double 	f, *dp;
	char 	*endptr;
	input_data  average_1_arg;

	average_1_arg.input_data.input_data_val = (double*) malloc(MAXAVGSIZE*sizeof(double));
	average_1_arg.sortType = sortType;

	dp = average_1_arg.input_data.input_data_val;
   
	average_1_arg.input_data.input_data_len = argc - 3;

	for( i = 2; i <= (argc - 2); i++ )
	{
		/* str to d ASCII string to floating point number */
		f = strtod( argv[i+1], &endptr);
		*dp = f;
		dp++;
	}

	clnt = clnt_create( host, AVERAGEPROG, AVERAGEVERS, "udp" );

	/* check if error */
	if (clnt == NULL) 
	{
		clnt_pcreateerror( host );
		exit(1);
	}

	//Get the sorted array
	average_1_arg = *average_1( &average_1_arg, clnt);

	if (average_1_arg.input_data.input_data_val == NULL) 
	{
		clnt_perror(clnt, "call failed:");
	}

	clnt_destroy( clnt );

	//Print the array and the sort type
	if (sortType == 1 )
	{
		printf("Array sorted in ascending order is:\n");
	}
	else if (sortType == -1)
	{
		printf("Array sorted in descending order is:\n");
	}
	int k =0;
	for (k = 0; k<argc-3; k++)
	{
		printf("%f\n",average_1_arg.input_data.input_data_val[k]);
	}
}


/* here is main */
main( int argc, char* argv[] )
{
	char *host;

	/* check correct syntax */
	if( argc < 4 )
	{
		printf( "usage: %s server_host sortType value ...\n", argv[0]); 
		exit(1);
	}

	if( argc > MAXAVGSIZE + 3 )
	{
		printf("Too many input values\n");
		exit(2);
	}

	if ( strcmp(argv[2], "ascending") == 0 || 
	strcmp(argv[2],"-a") == 0)
	{
		printf("Sorting the array in ascending order...\n");
		sortType = 1;
	}

	else if ( strcmp(argv[2], "descending") == 0 || 
	strcmp(argv[2],"-d") == 0)
	{
		printf("Sorting the array in descending order...\n");
		sortType = -1;
	}
	
	else
	{
		printf("Invalid sorting type, use -a or ascending for ascending and -d or descending for descending\n");
		exit(3);
	}

	/* host name is in first parameter (after program name) */
	host = argv[1];
	averageprog_1( host, argc, argv);
}
