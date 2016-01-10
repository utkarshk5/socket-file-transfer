//Code Contribution: Mythili Vutukuru
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h> 
#include <time.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

#define MAX_INPUT_SIZE 256
#define MAX_OUTPUT_SIZE 2097152

char* server_address;
int portnum, nthreads; float duration, sleep;
bool israndom;

void *handle(void *y){
	int *yptr = (int*)y; int x = *yptr;
	// Create the socket
	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd<0){
		fprintf(stderr, "Error opening socket for thread #%d\n", x);
		exit(1);
	}

	/* Fill in server address */
	struct sockaddr_in server_addr;
	bzero((char *) &server_addr, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	if(!inet_aton(server_address, &server_addr.sin_addr)) {
		fprintf(stderr, "ERROR invalid server IP address\n");
		exit(1);
	}
	server_addr.sin_port = htons(portnum);

	/* Connect to server */
	if (connect(sockfd,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0) {
		fprintf(stderr, "ERROR connecting to server\n");
		exit(1);
	}
	printf("Thread #%d connected to server\n", x);
	
	struct timeval then, now; float t1, t2;
	gettimeofday(&then, NULL); t1 = then.tv_sec+(then.tv_usec/1000000.0);
	char query[25];
	int n;
	do {
		if(israndom) sprintf(query, "get files/foo%d.txt", rand()%10000);
		else sprintf(query, "get files/foo0.txt");

		printf("Thread #%d querying \"%s\"\n", x, query);

		/* Write to server */
		n = write(sockfd,query,(int)strlen(query));
		if (n < 0) {
			fprintf(stderr, "Thread #%d ERROR writing to socket\n", x);
			exit(1);
		}

		/* Read reply */
		char recvbuf[2048], file_contents[MAX_OUTPUT_SIZE+5];
		bzero(recvbuf,2048);
		bzero(file_contents,MAX_OUTPUT_SIZE+5);
		
		n = recv(sockfd,recvbuf,10, MSG_WAITALL);
		while(n != 0){
			if (n < 0) {
				fprintf(stderr, "Thread #%d ERROR reading from socket\n", x);
				exit(1);
			}
			sprintf(file_contents, "%s%s", file_contents, recvbuf);
		}
		printf("Thread #%d received a message of size %d\n", x, (int)strlen(file_contents));
		gettimeofday(&now, NULL); t2 = now.tv_sec+(now.tv_usec/1000000.0);
	} while (t2 < t1+duration);
}

int main(int argc, char *argv[]) {

	if (argc < 7) {
	   fprintf(stderr,"usage %s <server-ip-addr> <server-port> <#threads> <duration> <sleep time> <mode>\n", argv[0]);
	   exit(0);
	}

	server_address = argv[1];
	portnum = atoi(argv[2]); nthreads = atoi(argv[3]);
	duration = (float)atoi(argv[4]); sleep = (float)atoi(argv[5]);
	israndom = (argv[6]=="random");

	srand(time(NULL));

	pthread_t threads[nthreads];
	int i;
	for(i=0; i<nthreads; i++){
		int ret = pthread_create(&threads[i], NULL, handle, &i);
		if(ret) {
			fprintf(stderr, "Error creating thread #%d\n", i);
			exit(1);
		}
	}

	for(i=0; i<nthreads; i++){
		pthread_join(threads[i], NULL);
	}
	return 0;
}
