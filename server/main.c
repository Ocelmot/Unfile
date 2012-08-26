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
#include <pthread.h>
#include <semaphore.h>
#include <netinet/tcp.h>

#include "connection_handler.h"
#include "btree.h"

#define MAX_THREADS 200

extern struct btree t;
int exit_flag=0;
sem_t threads;
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
	signal(SIGINT, save);
	signal(SIGSEGV, flushandquit);
	sem_init(&threads,0,MAX_THREADS);
	btree_init(&t);
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
	int flag=1;
	setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int)); 
	listen(sockfd,5);
	clilen = sizeof(cli_addr);
	
	signal(SIGPIPE, SIG_IGN);
	
	struct Connection *conn;
	pthread_t thread;
	while(exit_flag==0){
		sem_wait(&threads);
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
		if (newsockfd < 0) {
			fprintf(stderr,"ERROR on accept\n");
			continue;
		}
		printf("---Connection opened---\n");
		conn = malloc(sizeof(conn));
		conn->sock=newsockfd;
		conn->authenticated=0;
		pthread_create( &thread, NULL, handle_connection, (void*) conn);
	}
	
	close(sockfd);
	close(newsockfd);
	
	if(exit_flag==1){
		int i;
		sem_getvalue(&threads,&i);
		while(i!=MAX_THREADS){
			sem_getvalue(&threads,&i);
		}
	}
	
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