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
#define MAX_FIELD 20

//======== prototypes ========//
struct packet{
	unsigned int type;
	unsigned int size; 
    unsigned char source[MAX_NAME];    
    unsigned char msg[MAX_DATA];
// type:size:id:id,password
};

// get sockaddr, IPv4 or IPv6:

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

//======== helper functions ========//
void processPacket(struct packet data, char* packetInfo){
    
    sprintf(packetInfo, "%d:%d:%s:%s", data.type, data.size, data.source, data.msg);
    printf("%s\n", packetInfo);
    return;    
}

int main(int argc, char *argv[]) {

    //======== Variable Setup ========//
    int sockfd;
    int numbytes;
    char buf[MAXDATASIZE];
    char message[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    struct packet data;    

    char clientID[MAX_FIELD];
    char password[MAX_FIELD];
    char serverIP[MAX_FIELD];
    char serverPort[MAX_FIELD];     
    char messageBuf[MAX_DATA];
    char sendBuf[1000];

    int loggedIn = 0;

    fd_set readfds;
    //======== Connection Setup ========//

    // memset(&hints, 0, sizeof hints);
    // hints.ai_family = AF_UNSPEC;
    // hints.ai_socktype = SOCK_STREAM;

    // if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
    //     fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    //     return 1;
    // }

    // // loop through all the results and connect to the first we can
    // for (p = servinfo; p != NULL; p = p->ai_next) {
    //     if ((sockfd = socket(p->ai_family, p->ai_socktype,
    //             p->ai_protocol)) == -1) {
    //         perror("client: socket");
    //         continue;
    //     }


    //     if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
    //         close(sockfd);
    //         perror("client: connect");
    //         continue;
    //     }

    //     break;
    // }

    // if (p == NULL) {
    //     fprintf(stderr, "client: failed to connect\n");
    //     return 2;
    // }

    // inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr),
    //         s, sizeof s);
    // printf("client: connecting to %s\n", s);

    //======== Conference Loop ========//

    while (1) {


        FD_ZERO(&readfds);
        FD_SET(fileno(stdin),&readfds);

        if (sockfd > 0) {
            FD_SET(sockfd, &readfds);
            select(sockfd+1,&readfds,NULL,NULL,NULL);    	
        }else{
            select(fileno(stdin)+1,&readfds,NULL,NULL,NULL);    	
        }

        if (FD_ISSET(fileno(stdin),&readfds)){
            printf("input detected\n");  

            char command[MAX_DATA];
            // printf ("Enter Command or Message: \n");
            scanf("%s", command);
            
            if (command[0] == '/'){
                printf("you entered a command: %s\n", command);
                
                if (!strcmp(command,"/login")){
                    
                    if (loggedIn){
                        printf("Already logged in\n");
                        int ch;
                        while ((ch = getchar()) != '\n' && ch != EOF);
                        continue;
                    }
                    else{
                        //======== input and packet processing ========//
                        scanf(" %s %s %s %s", clientID, password, serverIP, serverPort);
                        printf("clientID: %s\n", clientID);
                        printf("password: %s\n", password);
                        printf("serverIP: %s\n", serverIP);
                        printf("serverPort: %s\n", serverPort);

                        strcpy(data.msg,password);
                        data.type = 0;
                        data.size = sizeof(password);
                        strcpy(data.source, clientID);

                        processPacket(data,sendBuf);

                        //======== Connection Setup ========//
                        memset(&hints, 0, sizeof hints);
                        hints.ai_family = AF_UNSPEC;
                        hints.ai_socktype = SOCK_STREAM;
                    
                        if ((rv = getaddrinfo(serverIP, serverPort, &hints, &servinfo)) != 0) {
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

                        //======== login request ========//

                        numbytes = send(sockfd,sendBuf,strlen(sendBuf),0);
                        printf("sendBytes: %d \n", numbytes);
                        

                        //======== login ack ========//
                        struct timeval tv;
                        tv.tv_sec = 0;
                        tv.tv_usec = 100;
                        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval));
                        
                        int numbytes=recv(sockfd, buf,sizeof (buf) , 0);
                        buf[numbytes] = '\0';
                
                        if (numbytes >= 0 && strcmp(buf,"ACK") >= 0){
                            printf("recieveBytes: %d \n", numbytes);
                            printf("message recieved: %s \n", buf);
                            loggedIn = 1;
                        }else{
                            printf("\n ACK not recieved \n\n");
                            loggedIn = 1; //=========================REMOVE==============================//
                        }
                        continue;
                    }
                    
                }
                else if(!strcmp(command,"/logout")){

                    if (loggedIn){
                        
                        continue;
                    }
                    else{
                        printf("Not logged in\n");
                        continue;
                    }

                }
                else if(!strcmp(command,"/joinsession")){
                    
                }
                else if(!strcmp(command,"/leavesession")){
                    
                }
                else if(!strcmp(command,"/createsession")){
                    
                }
                else if(!strcmp(command,"/list")){
                    
                }
                else if(!strcmp(command,"/quit")){
                    
                }

            }
            else{
                printf("you entered a message %s \n", command); 
                if (loggedIn){

                    strcpy(message,command);
                    if (!feof(stdin)){
                        gets(command);
                        strcat(message,command);
                    }
                    printf("message: %s\n", message);

                    numbytes = send(sockfd, message, strlen(message), 0);
                    printf("numbytes: %d\n", numbytes);
                
                }
                else{
                    printf("you are not logged in!\n");
                }        
            }
        }
        else if(loggedIn && FD_ISSET(sockfd,&readfds)){
            printf("Server sent something back\n");

            numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0);
            printf("numbytes: %d\n", numbytes);

            // if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
            //     perror("recv");
            //     exit(1);
            // }

            // printf("numbytes: %d\n", numbytes);
            buf[numbytes] = '\0';
            
            printf("client: received '%s'\n", buf);
        }
        else{
            // printf("whats happening anymore\n");
        }

        // send(sockfd, message, strlen(message), 0);
        // //freeaddrinfo(servinfo); // all done with this structure
	
        // if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
        //     perror("recv");
        //     exit(1);
        // }

        // buf[numbytes] = '\0';

        // printf("client: received '%s'\n", buf);
    }

    close(sockfd);

    return 0;
}

