/* written by client - calls client stub, xdr client, xdr xerver, server stub, server routine */ 

#include "avg.h"	/* header file generated by rpcgen */ 
#include <stdlib.h>


/* local routine client */
/* prototype can be whatever you want */
void averageprog_1( char* host, int argc, char *argv[] )
{
	CLIENT *clnt; /* client handle, rpc.h included in avg.h from rpcgen */
	int i;
	double 	f,  *result_1, *dp;
	char 	*endptr;
	input_data  average_1_arg; /* input_data rpc struct */

	average_1_arg.input_data.input_data_val = (double*) malloc(MAXAVGSIZE*sizeof(double));

	/* pointer to double, beginning of input data */
	dp = average_1_arg.input_data.input_data_val;

	/* set number of items */
	average_1_arg.input_data.input_data_len = argc - 2;

	for( i = 1; i <= (argc - 2); i++ )
	{
		/* str to d ASCII string to floating point nubmer */
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

	result_1 = average_1( &average_1_arg, clnt );

	if (result_1 == NULL) 
	{
		clnt_perror(clnt, "call failed:");
	}

	clnt_destroy( clnt );

	printf( "The median is: %f\n",*result_1 );
}

/* here is main */
main( int argc, char* argv[] )
{
	char *host;

	/* check correct syntax */
	if( argc < 3 )
	{
		printf( "usage: %s server_host value ...\n", argv[0]); 
		exit(1);
	}

	if( argc > MAXAVGSIZE + 2 )
	{
		printf("Two many input values\n");
		exit(2);
	}

	/* host name is in first parameter (after program name) */
	host = argv[1];
	averageprog_1( host, argc, argv);
}
