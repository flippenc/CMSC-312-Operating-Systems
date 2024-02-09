#include <rpc/rpc.h>
#include "avg.h"
#include <stdio.h>

//Server code for echo, takes a string "input" and returns it

input_data * average_1(input_data *input, CLIENT *client) 
{
	//printf("String from client is %s\n",input->input_data.input_data_val);
	return( input );
}
 
input_data * average_1_svc(input_data * input, struct svc_req *svc) 
{
	CLIENT *client;
	return( average_1(input, client) );
}
