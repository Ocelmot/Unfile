//Unfile Server
//by
//Chrys Adams

#include <stdio.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>

#include "connection_handler.h"
#include "btree.h"

extern struct btree t;
int exit_flag=0;
int sockfd, newsockfd;
char pass[] = "test";

void save();

void flushandquit(){
	printf("\nSegfault!\n");
	fflush(stdout);
	exit(0);
	return;
}


int main(int argc, char *argv[]){
	
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;
	printf("---Starting---\n");
	btree_init(&t);
	signal(SIGINT, save);
	signal(SIGSEGV, flushandquit);
	
	int portno=3000;
	
	
	printf("---Reading database...---\n");
	btree_load(&t,"hashes.dat");
	printf("---Reading done, listening...---\n");
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) 
		fprintf(stderr,"ERROR opening socket");
	memset((char *) &serv_addr,0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);
	
	if (bind(sockfd, (struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		fprintf(stderr,"Failed to bind, quitting.\n");
		exit(1);
	}
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	
	signal(SIGPIPE, SIG_IGN);
	
	struct Connection conn;
	while(!exit_flag){
		
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) {
			fprintf(stderr,"ERROR on accept\n");
		}
		printf("---Connection opened---\n");
		conn.sock=newsockfd;
		conn.authenticated=0;
		handle_connection(conn);
		printf("---Connection closed---\n");
	}
	
	close(sockfd);
	
	
	return 0;
}
void save(){
	printf("Saving database...\n");
	btree_save(&t,"hashes.dat");
	close(sockfd);
	close(newsockfd);
	exit(0);
	return;
}