#ifndef _CONNECTION_HANDLER
#define _CONNECTION_HANDLER
	
#include <stdlib.h>
#include <stdio.h>	
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <unistd.h>

struct Connection{
	int sock;
	int authenticated;
};


void *handle_connection(void *ptr);






#endif