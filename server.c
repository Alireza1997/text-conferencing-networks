/*
 ** selectserver.c -- a cheezy multiperson chat server
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define PORT "9034"   // port we're listening on
#define MAXUSERS 2
#define MAX_NAME 10
#define MAX_DATA 100

// get sockaddr, IPv4 or IPv6:


void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

struct lab3message {
	unsigned int type;
	unsigned int size;
	unsigned char source[10];
	unsigned char data[100];
};

void processPacket(int *type,int *size, char *source,char *data , char *buf);
char* preparePacket(struct lab3message message,  char* message_info);



int main(void) {
	
	const char * usernames[MAXUSERS];
	const char * passwords[MAXUSERS];
	char * session_ids[MAXUSERS]={""};
	 int file_id [MAXUSERS]={-1};
	usernames[0]="blah";
	usernames[1]="hi";
	passwords[0]="1";
	passwords[1]="2";
	struct lab3message message;
	
	
    fd_set master; // master file descriptor list
    fd_set read_fds; // temp file descriptor list for select()
    int fdmax; // maximum file descriptor number

    int listener; // listening socket descriptor
    int newfd; // newly accept()ed socket descriptor
    struct sockaddr_storage remoteaddr; // client address
    socklen_t addrlen;

    char buf[256]; // buffer for client data
    int nbytes;


    char remoteIP[INET6_ADDRSTRLEN];

    int yes = 1; // for setsockopt() SO_REUSEADDR, below
    int i, j, l,session, rv;
	int exists=0;
    int type;
    int size;
	char source [100];
	char data [100];	
	char *user;
	char *password;
	char user_session[100];		
	char message_info[100];	
	char users_online[100];
		

    struct addrinfo hints, *ai, *p;

    FD_ZERO(&master); // clear the master and temp sets
    FD_ZERO(&read_fds);

    // get us a socket and bind it
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if ((rv = getaddrinfo(NULL, PORT, &hints, &ai)) != 0) {
        fprintf(stderr, "selectserver: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = ai; p != NULL; p = p->ai_next) {
        listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (listener < 0) {
            continue;
        }

        // lose the pesky "address already in use" error message
        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof (int));

        if (bind(listener, p->ai_addr, p->ai_addrlen) < 0) {
            close(listener);
            continue;
        }

        break;
    }

    // if we got here, it means we didn't get bound
    if (p == NULL) {
        fprintf(stderr, "selectserver: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(ai); // all done with this

    // listen
    if (listen(listener, 10) == -1) {
        perror("listen");
        exit(3);
    }

    // add the listener to the master set
    FD_SET(listener, &master);

    // keep track of the biggest file descriptor
    fdmax = listener; // so far, it's this one

    // main loop
    for (;;) {
        read_fds = master; // copy it
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            perror("select");
            exit(4);
        }

        // run through the existing connections looking for data to read
        for (i = 0; i <= fdmax; i++) {

            if (FD_ISSET(i, &read_fds)) { // we got one!!
                if (i == listener) {

                    // handle new connections
                    addrlen = sizeof remoteaddr;
                    newfd = accept(listener,
                            (struct sockaddr *) &remoteaddr,
                            &addrlen);

                    if (newfd == -1) {
                        perror("accept");
                    } else {
                        FD_SET(newfd, &master); // add to master set
                        if (newfd > fdmax) { // keep track of the max
                            fdmax = newfd;
                        }
                        printf("selectserver: new connection from %s on "
                                "socket %d\n",
                                inet_ntop(remoteaddr.ss_family,
                                get_in_addr((struct sockaddr*) &remoteaddr),
                                remoteIP, INET6_ADDRSTRLEN),
                                newfd);
                    }
                } else {
                    // handle data from a client
                    if ((nbytes = recv(i, buf, sizeof buf, 0)) <= 0) {
                        // got error or connection closed by client
                        if (nbytes == 0) {
                            // connection closed
                            printf("selectserver: socket %d hung up\n", i);
                        } else {
                            perror("recv");
                        }
                        close(i); // bye!
                        FD_CLR(i, &master); // remove from master set
                    } else {
			processPacket(&type, &size, source, data,buf);
			switch(type){

				case (0):
					user=strtok(data,",");
					password=strtok(NULL,",");
					int k;
					for (k=0;k<MAXUSERS;k++){
						if (strcmp (usernames[k],user)==0)break;
					}
					if (strcmp(passwords[k],password)==0){
						//login ack
						message.type=1;		
						strcpy(message.source,source);
						strcpy(message.data,"");
						message.size=strlen(message.data);											
						// insert fd of this user into file_id 
						file_id[k]=i;
	
					}
					else{
						//login nack
						message.type=2;
						strcpy(message.source,source);
						strcpy(message.data,"incorrect password");
						message.size=strlen(message.data);											
										
					}
				//Prepare and send message
				preparePacket(message, buf);
				if (send(file_id[j], buf, nbytes, 0) == -1) {
					perror("send");
					}
				
				
				
				break;

				case(3):
					//exit
					
					close(i); // bye!
                        		FD_CLR(i, &master); // remove from master set
					break;
				case(4):
					//join
					//check if session exists:
					for(session=0;session<MAXUSERS;session++){
						if (strcmp (session_ids[session],data)==0){
							exists=1; 
							break;						
						}
						
					}
					
					//if exists, add session name to user's session id and send jn_ack
					if(exists){
						for (l=0;l<MAXUSERS;l++){
							if (strcmp (usernames[l],source)==0)break;
						}
						strcpy(session_ids[l],data);
						message.type=5;
						strcpy(message.source,source);
						strcpy(message.data,data);
						message.size=strlen(message.data);
					}
					//if doesnt exists, send jn_nack
					else{
						message.type=6;
						strcpy(message.source,source);
						strcpy(message.data,data);
						strcat(message.data,",Session does not exists");
						message.size=strlen(message.data);
					}

					break;
				case(7):
					//leave session, get user id and delete their current session id
					for (l=0;l<MAXUSERS;l++){
							if (strcmp (usernames[l],source)==0)break;
					}
					strcpy(session_ids[l],"");	
					
					break;
				case(8):
					//add new session, get user id and add the session id
					for (l=0;l<MAXUSERS;l++){
							if (strcmp (usernames[l],source)==0)break;
					}
					strcpy(session_ids[l],data);	
					//send ack
					message.type=9;
					strcpy(message.source,source);
					strcpy(message.data,data);
					message.size=strlen(message.data);
					
					break;
		


				case(10):
					//send message
					message.type=10;
					strcpy(message.source,source);
					strcpy(message.data,data);
					message.size=strlen(message.data);
					preparePacket(message, buf);
					// get user id
					for (l=0;l<MAXUSERS;l++){
							if (strcmp (usernames[l],source)==0)break;
					}
					//get session_id of this user
					strcpy(user_session,session_ids[l]);
					
					// send message to all users with this session id

					for (j=0;j<MAXUSERS;j++){
						if(strcmp (session_ids[j],user_session)==0){
 							if (FD_ISSET(file_id[j], &master)) {
								if(file_id[j]!=listener &&file_id[j]!=i){
									if (send(file_id[j], buf, nbytes, 0) == -1) {
										perror("send");
										}
									
								}

							}
						}				
					}

					break;

				case(11):
					//quack
					message.type=12;
					strcpy(message.source,source);
					for (l=0;l<MAXUSERS;l++){
							if (file_id[l]!=-1){
								strcat(message.data,usernames[l]);
								strcat(message.data,session_ids[l]);
							}
					}
					message.size=strlen(message.data);
					
					break;
		

			}
			// if it is not a broadcasting message, send packet back to user
			if(type!=10){
			//Prepare and send message to user
				preparePacket(message, buf);
				if (send(i, buf, nbytes, 0) == -1) {
					perror("send");
					}
			}
	


                    }
                } // END handle data from client
            } // END got new incoming connection
        } // END looping through file descriptors
    } // END for(;;)--and you thought it would never end!

    return 0;
}



void processPacket(int *type,int *size, char *source,char *data , char *buf){
	int i, j = 0;
	for (i = 0; buf[i]!= ':'; i++){
		*type *=10;
		*type += buf[i] - '0';
	}

	for (i = i+1; buf[i]!= ':'; i++){
		*size *=10;
		*size += buf[i] - '0';
	}j = i;
	for (i = i+1; buf[i]!= ':'; i++){
		source[i-j-1] = buf[i]; 				
		//printf("%s \n", filename);
	}
	data[i-j-1] = '\0';
	j = i;
	for (i = i+1; i-j <= *size; i++){
		data[i-j-1] = buf[i]; 				
		//printf("%u \n", buf[i]);
	}

}



char* preparePacket(struct lab3message message,  char* message_info){

	char buffer [200];
	sprintf(buffer,"%d:",message.type);
	strcat(message_info, buffer);


	sprintf(buffer,"%d:",message.size);
	strcat(message_info, buffer);

 
	sprintf(buffer,"%d:",message.source);
	strcat(message_info, buffer);


	strcat(message_info, message.data);

	

	return message_info;

}



