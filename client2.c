/*
 ** client.c -- a stream socket client demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define PORT "9034" // the port client will be connecting to 

#define MAXDATASIZE 100 // max number of bytes we can get at once 
#define MAX_NAME 10
#define MAX_DATA 100

// get sockaddr, IPv4 or IPv6:

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*) sa)->sin6_addr);
}
struct lab3message{

	unsigned char type [MAX_DATA];
	unsigned int size;
	unsigned char source[MAX_NAME];
	unsigned char data[MAX_DATA];
};

int main(int argc, char *argv[]) {
	char hello [MAX_DATA];
    	int sockfd;
        int numbytes;
	char command [MAXDATASIZE];

        char buf[MAXDATASIZE];
 	char port[MAXDATASIZE];
 	char ip[MAXDATASIZE];
 	char client_id[MAXDATASIZE];
 	char client_password[MAXDATASIZE];
 	char session_id[MAXDATASIZE];
	char text[MAXDATASIZE];
        struct addrinfo hints, *servinfo, *p;
	struct lab3message message;
        int rv;
        char s[INET6_ADDRSTRLEN];
	

	scanf("%s",command);

	if (strcmp(command,"/login")==0){
		scanf("%s",client_id);
		scanf("%s",client_password);
		scanf("%s",ip);		
		scanf("%s",port);
		strcpy(message.type, "0");
		strcpy(message.source, client_id);

		strcpy(message.data,client_id);
		strcat(message.data," ");
		strcat(message.data,client_password);
		message.size=sizeof(message.data);



	}



        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_UNSPEC;
        hints.ai_socktype = SOCK_STREAM;

        if ((rv = getaddrinfo(ip, port, &hints, &servinfo)) != 0) {
            fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
            return 1;
        }

        // loop through all the results and connect to the first we can
        for (p = servinfo; p != NULL; p = p->ai_next) {
            if ((sockfd = socket(p->ai_family, p->ai_socktype,
                    p->ai_protocol)) == -1) {
                perror("client: socket");
                continue;
            }


            if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
                close(sockfd);
                perror("client: connect");
                continue;
            }

            break;
        }

        if (p == NULL) {
            fprintf(stderr, "client: failed to connect\n");
            return 2;
        }

        inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr),
                s, sizeof s);
        printf("client: connecting to %s\n", s);
//recv(sockfd,hello, sizeof(hello),0);
send(sockfd, message.type, strlen(message.type),0);
send(sockfd, message.data, strlen(message.data),0);
//send(sockfd, message.data, sizeof(message.data), 0);
     //   printf("%s\n", "hello");

while (1){
	//send(sockfd, "hello", strlen("hello"), 0);
//send(sockfd, message.data, sizeof(message.data), 0);
        //printf("%s\n", "hello");
	//if( send(sockfd, message.type, strlen(message.type), 0)==-1) printf("%s\n","error");
	 //if( send(sockfd, message.data, strlen(message.data), 0)==-1) printf("%s\n","error");

	
	}

       

}
