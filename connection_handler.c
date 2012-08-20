#include "connection_handler.h"
#include "mesg.h"
#include "btree.h"

struct btree t;

struct mesg* parse_input(struct Connection conn);
int handle_message(struct Connection conn, struct mesg* msg);

extern char pass[];
extern int exit_flag;

void handle_connection(struct Connection conn){

	int status;
	struct mesg* msg=NULL;
	
	
	while(1){
		msg=parse_input(conn);
		if(msg==NULL){
			break;
		}
		
		status=handle_message(conn,msg);
		if (status==1){	
			free(msg->data);
			free(msg);
			break;
		}
		
		
		free(msg->data);
		free(msg);
	}

	close(conn.sock);
}
struct mesg* parse_input(struct Connection conn){

	int n;
	int i;
	int bytes=0;
	unsigned int buf;
	char buffer[256];

	fd_set set;
	FD_ZERO(&set);
	FD_SET(conn.sock, &set);
	struct timeval timeout;
	int select_result;
	
	struct mesg* msg=malloc(sizeof(struct mesg));
	msg->len=0;
	msg->command=0;
	msg->subcommand=0;
	msg->data=NULL;
	while(1){
		
		
		
		
		memset(buffer,0,255);
		timeout.tv_sec = 30;
		timeout.tv_usec = 0;
		
		select_result = select(conn.sock + 1, &set, NULL, NULL, &timeout);
		if(select_result<0){
			msg->len=0;
			msg->command=0;
			msg->subcommand=3;
			msg->data=NULL;
			return msg;
		}else if(select_result==0){
			msg->len=0;
			msg->command=0;
			msg->subcommand=1;
			msg->data=NULL;
			return msg;
		}else{
			if(bytes<6){			//Fill message head
				n = read(conn.sock,buffer,6-bytes);
				if (n < 0){
					msg->len=0;
					msg->command=0;
					msg->subcommand=2;
					msg->data=NULL;
					return msg;
				}
				if (n==0){
					msg->len=0;
					msg->command=0;
					msg->subcommand=0;
					msg->data=NULL;
					return msg;
				}
				i=0;
				switch(bytes){
				case 0:
					buf=buffer[i];
					buf=buf<<24;
					msg->len=msg->len|buf;
					i++;
					if(i==n){
						break;
					}
				case 1:
					buf=buffer[i];
					buf=buf<<16;
					msg->len=msg->len|buf;
					i++;
					if(i==n){
						break;
					}
				case 2:
					buf=buffer[i];
					buf=buf<<8;
					msg->len=msg->len|buf;
					i++;
					if(i==n){
						break;
					}
				case 3:
					buf=buffer[i];
					msg->len=msg->len|buf;
					i++;
					if(i==n){
						break;
					}
				case 4:
					msg->command=buffer[i];
					i++;
					if(i==n){
						break;
					}
				case 5:
					msg->subcommand=buffer[i];
					i++;
					if(i==n){
						break;
					}
				}
				bytes+=n;
				if(bytes==6){
					if(msg->len==0){
						break;
					}
					msg->data=malloc(msg->len);
				}
				continue;
			}
			if(bytes>=6){			//fill message tail
				
				if((msg->len-(bytes-6))>256){
					n = read(conn.sock,buffer,256);
				}else{
					n = read(conn.sock,buffer,(msg->len-(bytes-6)));
				}
				memcpy(&(msg->data[bytes-6]),buffer,n);
				bytes+=n;
				if((bytes-6)==msg->len){
					break;
				}
			}	
		}
	}
	return msg;
}

int handle_message(struct Connection conn, struct mesg* msg){
	
	int n;
	int result;
	
	switch(msg->command){
		case 0:
			switch(msg->subcommand){
			case 0:
				printf("Client Quit\n");
				return 1;
			case 1:
				printf("Client Timed Out\n");
				return 1;
			case 2:
				printf("Socket Error\n");
				return 1;
			case 3:
				printf("Select() Error\n");
				return 1;
			default:
				printf("Client Quit, Unknown Error.\n");
				return 1;
			}
		case 1:
			if(msg->len!=20){
				return 2;
			}
			switch(msg->subcommand){
				case 0:
					result=btree_inc(&t,msg->data);
					n = write(conn.sock,"\x04" "\x00" "\x00" "\x00" "\x01" "\x00",6);
					if (n < 0){
						fprintf(stderr,"ERROR writing to socket\n");
						return 1;
					}
					result = __builtin_bswap32(result);
					n = write(conn.sock,&result,4);
					if (n < 0){
						fprintf(stderr,"ERROR writing to socket\n");
						return 1;
					}
					return 0;
				case 1:
					result=btree_get(&t,msg->data);
					n = write(conn.sock,"\x04" "\x00" "\x00" "\x00" "\x01" "\x00",6);
					if (n < 0){
						fprintf(stderr,"ERROR writing to socket\n");
						return 1;
					}
					result = __builtin_bswap32(result);
					n = write(conn.sock,&result,4);
					if (n < 0){
						fprintf(stderr,"ERROR writing to socket\n");
						return 1;
					}
					return 0;
				default:
					return 0;
			}
		case 2:
			if(strcmp(msg->data,pass)==0){
				n=write(conn.sock,"\0" "\0" "\0" "\0" "2" "1",6);
				if (n < 0){
					return 1;
				}
				conn.authenticated=1;
			}else{
				n=write(conn.sock,"\0" "\0" "\0" "\0" "2" "0",6);
				if (n < 0){
					return 1;
				}
			}
			return 0;
		case 10:
			switch(msg->subcommand){
				case 1:
					//save database
					exit_flag=1;
					break;
				case 2:
					//save database
					break;
			}
	}
	return 1;
}