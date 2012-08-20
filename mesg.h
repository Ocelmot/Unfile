#ifndef _MESG_H
#define _MESG_H
	
struct mesg{
	unsigned int len;
	unsigned char command;
	unsigned char subcommand;
	char* data;
};


#endif