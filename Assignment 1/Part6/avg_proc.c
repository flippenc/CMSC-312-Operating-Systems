#include <rpc/rpc.h>
#include "avg.h"
#include <stdio.h>
#include <math.h>
/**
 * Sorting information found from:
 * https://stackoverflow.com/questions/8448790/sort-arrays-of-double-in-c
 * One compare function is used for ascending, the other for descending order
 * Both functions turn the void pointer parameters into doubles, then the doubles
 * are compared and the return values represent which is larger
 */
int cmpfuncAscend(const void * a, const void * b) 
{
	double aa = *(double*)a, bb = *(double*)b;
	if(aa <bb)
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

int cmpfuncDescend(const void * a, const void *b)
{
	double aa = *(double*)a, bb = *(double*)b;
        if(aa > bb)
        {
                return -1;
        }
        if (aa < bb)
        {
                return 1;
        }
	else
	{
        	return 0;
	}
}

input_data * average_1(input_data *input, CLIENT *client) 
{

	double *dp = input->input_data.input_data_val;
	int sortType = input->sortType;
	u_int i;

	double dataArray[input->input_data.input_data_len]; 

	//Putting the input into an array
	for( i = 1; i <= input->input_data.input_data_len; i++ )
	{
		dataArray[i-1] = *dp;  
    		dp++;
	}

	//Sorting the array
	//sortType 1 represents ascending, -1 represents descending
	if (sortType == 1)
	{
		qsort(dataArray, input->input_data.input_data_len, sizeof(double), cmpfuncAscend);
	}
	else if (sortType == -1)
	{
		qsort(dataArray, input->input_data.input_data_len, sizeof(double), cmpfuncDescend);
	}

	//Test code for printing the received array
	//int z =0;
	//for (z= 0; z<input->input_data.input_data_len; z++)
	//{
	//	printf("dataArray contains: %f\n",dataArray[z]);
	//}

	//Putting the array back into the input_data struct
	int j = 0;
	for (j=0; j<input->input_data.input_data_len; j++)
	{
		input->input_data.input_data_val[j] = dataArray[j];	
	}
	//Returning the input_data struct with the sorted array
	return( input );
}
 
input_data * average_1_svc(input_data *input, struct svc_req *svc)
{
	CLIENT *client;
	return( average_1( input, client) );
}
