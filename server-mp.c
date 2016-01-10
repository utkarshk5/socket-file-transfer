#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_INPUT_SIZE 1024
#define MAX_OUTPUT_SIZE 2097152
int BACKLOG = 10;

/* Signal handler to reap zombie processes */
static void wait_for_child(int sig) {
	while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle(int sock) {
	/* recv(), send(), close() */

	char outputbuf[MAX_OUTPUT_SIZE+5];
	char inputbuf[MAX_INPUT_SIZE];

	bzero(&inputbuf, MAX_INPUT_SIZE);
	int FLG=0;
	if(recv(sock,inputbuf,MAX_INPUT_SIZE,FLG) <= 0){
		perror("recv");
	}
	else{
		printf("Received msg: \"%s\"\n", inputbuf);
		if(inputbuf[0]=='g' && inputbuf[1]=='e' && inputbuf[2]=='t' && inputbuf[3]==' '){
			// figure out the file in  consideration and write its contents (MAX_OUTPUT_SIZE characters) to outputbuf
			char filename[10];
			strncpy(filename, &inputbuf[4], strlen(inputbuf)-4);
			filename[strlen(inputbuf)-4] = '\0';
			printf("File to be processed is \"%s\"\n", filename);
			FILE *file_to_read = fopen(filename, "rb");
			if(file_to_read == NULL){
				perror("file not found");
				exit(3);
			}
			bzero(&outputbuf, MAX_OUTPUT_SIZE+5);
			fread(outputbuf, sizeof(char), MAX_OUTPUT_SIZE, file_to_read);
			printf("outputbuf is \"%s\"\n and length is %d\n", outputbuf, (int)strlen(outputbuf));
			outputbuf[strlen(outputbuf)] = '\0';
			fclose(file_to_read);

			int bytes_sent = send(sock, outputbuf, MAX_OUTPUT_SIZE, 0);
			if(bytes_sent != MAX_OUTPUT_SIZE){
				perror("file not sent");
				exit(3);
			}
		}

		else{
			sprintf(outputbuf,"Not a valid msg");
			int len, bytes_sent;
			len = strlen(outputbuf);
			printf("%s \n",outputbuf);
			bytes_sent = send(sock, outputbuf, len, 0);
		}  
	}
	close(sock);
}

int main(int argc, char *argv[]){
	int sockfd;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;

	struct sigaction sa;

	//handling the errors in arguments to program
	if (argc != 2) {
		//check for number of arguments
		fprintf(stderr,"usage: %s <tcp-port-number>\n", argv[0]);
		exit(1);
	}
	else{
		int i=0;
		if(argv[1][0] == '-'){
			//check fornegative number
			fprintf(stderr,"Invalid argument for <tcp-port-number>\n");
			exit(1);
		}
		for(; argv[1][i] != 0; i++){
			if(!isdigit(argv[1][i])){
				//check for integer value
				fprintf(stderr,"Invalid argument for <tcp-port-number>\n");
				exit(1);
		   }
	   }
	}

	int MYPORT = atoi(argv[1]);
	
	//creating socket
	sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd==-1) perror("socket");
	my_addr.sin_family = AF_INET;
	my_addr.sin_port = htons(MYPORT);
	my_addr.sin_addr.s_addr = INADDR_ANY;
	printf("Address is %s \n",inet_ntoa(my_addr.sin_addr));
	memset(&(my_addr.sin_zero), '\0', 8);
	int yes=1;

	//making the port reusable
	if (setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
		perror("setsockopt");
		exit(3);
	}

	//bind
	if(bind(sockfd, (struct sockaddr *)&my_addr, sizeof(struct sockaddr))==-1){
		perror("bind");
		exit(2);
	}

	//listening on port
	listen(sockfd,BACKLOG);

	//accepting connection
	int sin_size = sizeof(struct sockaddr_in);


	// on accepting, spawn a child process and hand over the new_fd to it which will then take care of the client

	//receiving message continuously
	while(1){

		int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if(new_fd== -1){
			perror("accept");
			exit(3);
		}

		int pid;
		printf("Accepted a connection by %s:%d\n", inet_ntoa(their_addr.sin_addr), htons(their_addr.sin_port));

		pid = fork();
		if(pid==0){
			// in child process now
			handle(new_fd);
			return 0;
		} else {
			// in parent process now
			if(pid==-1){
				perror("fork");
				return 1;
			}
		}
	}
	return 0;
}
