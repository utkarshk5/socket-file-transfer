#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define MAX_INPUT_SIZE 1024
#define SECTOR_SIZE 2048

int BACKLOG = 10;

int main(int argc, char *argv[]){
	int sockfd;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;

	char recvbuf[MAX_INPUT_SIZE];
	char sendbuf[SECTOR_SIZE];

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
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
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
	int new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
	if(new_fd== -1){
		perror("accept");
		exit(3);
	}
	else close(sockfd); //close once one connection is accepted
	//receiving message continuously
	while(1){
		bzero(recvbuf, MAX_INPUT_SIZE);
		int bytes_recv = read(new_fd,recvbuf,MAX_INPUT_SIZE);
		if(bytes_recv < 0){
			perror("recv");
		} else{
			printf("Received msg: \"%s\"\n", recvbuf);
			if(recvbuf[0]=='g' && recvbuf[1]=='e' && recvbuf[2]=='t' && recvbuf[3]==' '){
				// figure out the file in  consideration and write its contents (MAX_OUTPUT_SIZE characters) to outputbuf
				char filename[25];
				strncpy(filename, &recvbuf[4], strlen(recvbuf)-4);
				filename[strlen(recvbuf)-4] = '\0';
				printf("File to be processed is \"%s\"\n", filename);
				FILE *file_to_read = fopen(filename, "rb");
				if(file_to_read == NULL){
					perror("file not found");
					exit(3);
				}
				bzero(&sendbuf, SECTOR_SIZE);
				int bytes_read = fread(sendbuf, 1, SECTOR_SIZE, file_to_read), x;
				while(bytes_read>0) {
					x = write(new_fd, sendbuf, bytes_read);
					bzero(&sendbuf, SECTOR_SIZE);
					bytes_read = fread(sendbuf, 1, SECTOR_SIZE, file_to_read);
				}
				close(new_fd);
				fclose(file_to_read);
			}
			else{
				sprintf(sendbuf,"OK %s:%d",inet_ntoa(their_addr.sin_addr),ntohs(their_addr.sin_port));
				printf("%s\n",sendbuf);
				int bytes_sent = send(new_fd, sendbuf, strlen(sendbuf), 0);
			}  
		 }
	}
	return 0;
}
