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

//======== Type specs ========//

#define LOGIN       0
#define LO_ACK      1
#define LO_NAK      2
#define EXIT        3
#define JOIN        4
#define JN_ACK      5
#define JN_NAK      6
#define LEAVE_SESS  7
#define NEW_SESS    8
#define NS_ACK      9
#define MESSAGE     10
#define QUERY       11
#define QU_ACK      12
#define INVITE      13
#define QU_INV      14

//======== prototypes ========//
struct packet{
	unsigned int type;
	unsigned int size; 
    unsigned char source[MAX_NAME];    
    unsigned char msg[MAX_DATA];
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

void deProcessPacket(int *type,int *size, char *source,	char *msg, char* buf){
    *type = 0; *size = 0;
    memset(source,0,strlen(source));	
	memset(msg,0,strlen(msg));
    int i, j = 0;
	for (i = 0; buf[i]!= ':'; i++){
		*type *=10;
		*type += buf[i] - '0';
	}
	for (i = i+1; buf[i]!= ':'; i++){
		*size *=10;
		*size += buf[i] - '0';
	}
	j = i;
	for (i = i+1; buf[i]!= ':'; i++){
		source[i-j-1] = buf[i]; 				
		//printf("%s \n", filename);
	}
	source[i-j-1] = '\0';
	j = i;
	for (i = i+1; i-j <= *size; i++){
		msg[i-j-1] = buf[i]; 				
		//printf("%u \n", buf[i]);
    }
    msg[i-j-1] = '\0';
    memset(buf,0,strlen(buf));	
	// memset(data,0,strlen(data));
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
    struct packet reData;    

    char clientID[MAX_FIELD];
    char sessionID[MAX_FIELD];
    char password[MAX_FIELD];
    char serverIP[MAX_FIELD];
    char serverPort[MAX_FIELD];     
    char messageBuf[MAX_DATA];
    char sendBuf[1000];

    int loggedIn = 0;

    fd_set readfds;
    

    //======== Conference Loop ========//
    while (1) {

        //======== select setup ========//
        FD_ZERO(&readfds);
        FD_SET(fileno(stdin),&readfds);

        if (sockfd > 0) {
            FD_SET(sockfd, &readfds);
            select(sockfd+1,&readfds,NULL,NULL,NULL);    	
        }else{
            select(fileno(stdin)+1,&readfds,NULL,NULL,NULL);    	
        }

        //======== Responses to keyboard input ========//
        if (FD_ISSET(fileno(stdin),&readfds)){
            printf("input detected\n");  

            char command[MAX_DATA];
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
                        data.type = LOGIN;
                        data.size = strlen(password);
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
                                                
                        continue;
                    }
                    
                }
                else if(!strcmp(command,"/logout")){

                    if (loggedIn){
                        close(sockfd);
                        loggedIn = 0;
                        continue;
                    }
                    else{
                        printf("Not logged in\n");
                        continue;
                    }

                }
                else if(!strcmp(command,"/joinsession")){

                    if (loggedIn){
                        //======== input and packet processing ========//
                        scanf("%s", sessionID);
                        
                        strcpy(data.msg,sessionID);
                        data.type = JOIN;
                        data.size = strlen(sessionID);
                        strcpy(data.source, clientID);

                        processPacket(data,sendBuf);

                        //======== request ========//

                        numbytes = send(sockfd,sendBuf,strlen(sendBuf),0);
                        printf("sendBytes: %d \n", numbytes);
                        continue;
                    }
                    else{
                        printf("Not logged in\n");
                        continue;
                    }
                    
                }
                else if(!strcmp(command,"/leavesession")){
                    if (loggedIn){
                        //======== input and packet processing ========//

                        strcpy(data.msg,"");
                        data.type = LEAVE_SESS;
                        data.size = strlen(sessionID);
                        strcpy(data.source, clientID);

                        processPacket(data,sendBuf);

                        //======== request ========//

                        numbytes = send(sockfd,sendBuf,strlen(sendBuf),0);
                        printf("sendBytes: %d \n", numbytes);
                        continue;
                    }
                    else{
                        printf("Not logged in\n");
                        continue;
                    }                    
                }
                else if(!strcmp(command,"/createsession")){
                    if (loggedIn){
                        //======== input and packet processing ========//
                        scanf("%s", sessionID);
                        
                        strcpy(data.msg,sessionID);
                        data.type = NEW_SESS;
                        data.size = strlen(sessionID);
                        strcpy(data.source, clientID);

                        processPacket(data,sendBuf);

                        //======== request ========//

                        numbytes = send(sockfd,sendBuf,strlen(sendBuf),0);
                        printf("sendBytes: %d \n", numbytes);
                        continue;
                    }
                    else{
                        printf("Not logged in\n");
                        continue;
                    }  
                }
                else if(!strcmp(command,"/list")){
                    if (loggedIn){
                        //======== input and packet processing ========//

                        strcpy(data.msg,"");
                        data.type = QUERY;
                        data.size = strlen(sessionID);
                        strcpy(data.source, clientID);

                        processPacket(data,sendBuf);

                        //======== request ========//

                        numbytes = send(sockfd,sendBuf,strlen(sendBuf),0);
                        printf("sendBytes: %d \n", numbytes);
                        continue;
                    }
                    else{
                        printf("Not logged in\n");
                        continue;
                    }
                }
                else if(!strcmp(command,"/quit")){
                    printf("bye!");
                    exit(0);
                }
                else if(!strcmp(command,"/invite")){
                    printf("invite command \n");
                    if (loggedIn){
                        char inviteID[MAX_FIELD];
                        
                        //======== input and packet processing ========//
                        scanf("%s", inviteID);
                        
                        strcpy(data.msg,inviteID);
                        data.type = INVITE;
                        data.size = strlen(inviteID);
                        strcpy(data.source, clientID);

                        processPacket(data,sendBuf);

                        //======== request ========//

                        numbytes = send(sockfd,sendBuf,strlen(sendBuf),0);
                        printf("sendBytes: %d \n", numbytes);
                        continue;
                    }
                    else{
                        printf("Not logged in\n");
                        continue;
                    }
                    
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

                    //======== input and packet processing ========//

                    strcpy(data.msg,message);
                    data.type = MESSAGE;
                    data.size = strlen(data.msg);
                    strcpy(data.source, clientID);

                    processPacket(data,sendBuf);
                    
                    //======== request ========//
                    numbytes = send(sockfd, sendBuf, strlen(sendBuf), 0);
                    printf("numbytes: %d\n", numbytes);
                
                }
                else{
                    printf("you are not logged in!\n");
                }        
            }
        }//======== Responses to server replies ========//
        else if(FD_ISSET(sockfd,&readfds)){
            printf("Server sent something back\n");
            numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0);
            printf("numbytes: %d\n", numbytes);
            buf[numbytes] = '\0';            
            printf("client: received '%s'\n", buf);
            deProcessPacket(&reData.type,&reData.size, reData.source,	reData.msg, buf);
            printf("type: %d\n", reData.type);
            printf("size: %d\n", reData.size);
            printf("source: %s\n", reData.source);
            printf("msg: %s\n", reData.msg);

            switch (reData.type){   
                case(LO_ACK):
                    printf("login successful!\n");
                    loggedIn = 1;
                    break;           
                case(LO_NAK):
                    printf("login failed!\n");
                    break;        
                case(JN_ACK):
                    printf("join successful!\n");
                    break;      
                case(JN_NAK):
                    printf("join failed!\n");
                    break;      
                case(NS_ACK):
                    printf("New Session Made!\n");
                    break;      
                case(MESSAGE):
                    printf("%s: %s\n", reData.source, reData.msg);
                    break;        
                case(QU_ACK):
                    printf("%s\n", reData.msg);
                    break;
                case(QU_INV):
                                        
                    //======== input and packet processing ========//
                    do{
                        printf("join session %s? (Y/N)\n", reData.msg);
                        scanf("%s", messageBuf);
                    }while(!strcmp("Y",messageBuf) && !strcmp("N",messageBuf));

                    if (!strcmp("N",messageBuf))
                        break;

                    strcpy(data.msg,reData.msg);
                    data.type = JOIN;
                    data.size = strlen(data.msg);
                    strcpy(data.source, clientID);

                    processPacket(data,sendBuf);

                    //======== request ========//

                    numbytes = send(sockfd,sendBuf,strlen(sendBuf),0);
                    printf("sendBytes: %d \n", numbytes);
                    break;
                default:
                    printf("invalid reply\n");
            }
                

        }
        else{
            // printf("whats happening anymore\n");
        }
    }

    return 0;
}

