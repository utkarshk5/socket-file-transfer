//Code Contribution: Mythili Vutukuru
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_SIZE 256
#define MAX_OUTPUT_SIZE 2097152
#define SECTOR_SIZE 2048

int main(int argc, char *argv[])
{
	int sockfd, portnum, n;
	struct sockaddr_in server_addr;

	char inputbuf[MAX_INPUT_SIZE];
	char outputbuf[MAX_OUTPUT_SIZE+5];
	char recvbuf[SECTOR_SIZE];

	if (argc < 3) {
	   fprintf(stderr,"usage %s <server-ip-addr> <server-port>\n", argv[0]);
	   exit(0);
	}

	portnum = atoi(argv[2]);

	/* Create client socket */
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		fprintf(stderr, "ERROR opening socket\n");
		exit(1);
	}

	/* Fill in server address */
	bzero((char *) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	if(!inet_aton(argv[1], &server_addr.sin_addr)) {
		fprintf(stderr, "ERROR invalid server IP address\n");
		exit(1);
	}
	server_addr.sin_port = htons(portnum);

	/* Connect to server */
	if (connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
		fprintf(stderr, "ERROR connecting\n");
		exit(1);
	}
	printf("Connected to server\n");
	
	do {
		/* Ask user for message to send to server */
		printf("Please enter the message to the server: ");
		bzero(inputbuf,MAX_INPUT_SIZE);
		fgets(inputbuf,MAX_INPUT_SIZE-1,stdin);
		inputbuf[strlen(inputbuf)-1] = '\0';
		printf("Sending msg \"%s\"\n", inputbuf);

		/* Write to server */
		n = write(sockfd,inputbuf,(int)strlen(inputbuf));
		if (n < 0) {
			fprintf(stderr, "ERROR writing to socket\n");
			exit(1);
		}
		
		/* Read reply */
		bzero(outputbuf,MAX_OUTPUT_SIZE+5);
		bzero(recvbuf,SECTOR_SIZE);
		int count = 0;
		while((n = read(sockfd,recvbuf,SECTOR_SIZE, 0)) > 0){
			printf("begin %d %d \n", ++count, n);
			printf("%d bytes read from buffer and %d bytes written to buffer\n", n, (int)strlen(recvbuf));
			if (n < 0) {
				fprintf(stderr, "ERROR reading from socket\n");
				exit(1);
			}
			bzero(recvbuf,SECTOR_SIZE);
			printf("end %d\n", count);
		}
		printf("received the complete message\n");
	} while (1);
	return 0;
}
