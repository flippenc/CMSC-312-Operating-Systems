#include <rpc/rpc.h>
#include "avg.h"
#include <stdio.h>
#include <math.h>
/**
 * Sorting information found from:
 * https://stackoverflow.com/questions/8448790/sort-arrays-of-double-in-c
 * The compare function is used by the qsort function to compare doubles,
 * similarly to Comparator in Java
 */
int cmpfunc (const void * a, const void * b) 
{
	//Get doubles from the void pointer parameters
	double aa  = *(double*)a, bb = *(double*)b;
	
	//Compare the doubles, return 1 if aa>bb, 
	//-1 if aa<bb, and 0 if they are equal
	if (aa < bb)
	{
		return -1;
	}
	if (aa > bb)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/* run locally on 'server' called by a remote client. */
static double median;

/* 
 * routine notice the _1 the version number 
 * notice the client handle, not sued here but needs to be 
 * a parameter 
 */
double * average_1(input_data *input, CLIENT *client) 
{
/* input is paramters were marshalled by genrated routine */
/*  a pointer to a double, is set to begining of data array  */
	double *dp = input->input_data.input_data_val;

	u_int i;
	median = 0;

	/* iterate until end of number of times (data_len) */
	double dataArray[input->input_data.input_data_len]; 

	for( i = 1; i <= input->input_data.input_data_len; i++ )
    	{
		dataArray[i-1] = *dp;  
		printf("dataArray[%d-1] is %lf\n",i,dataArray[i-1]);
    		dp++;
    	}
	//Sort the array, then get the median values from it
	qsort(dataArray, input->input_data.input_data_len, sizeof(double), cmpfunc);

	//If an array has an even number of elements, the two middle ones are averages to get the median
 	if (input->input_data.input_data_len % 2 == 0)
	{
		median = (dataArray[input->input_data.input_data_len/2 -1]+dataArray[input->input_data.input_data_len/2])/2.0;
	}
	//If an array has an odd number of elements, the middle element is the median
	else
	{
		median = dataArray[input->input_data.input_data_len/2];
	}
	return( &median );
}
 
double * average_1_svc(input_data *input, struct svc_req *svc) 
{
	CLIENT *client;
	return( average_1( input, client) );
}
