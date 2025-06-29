#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#define MAXUSERS 1000
#define MAXLEN 1000

int numUsers = 0;
int count = 0;
char usernames[MAXUSERS][MAXLEN];
int clientfds[MAXUSERS];

//Function to exit program when an error has occurred
void pexit(char *errmsg){
	fprintf(stderr, "%s\n", errmsg);
	exit(1);
}

//Function to handle messaging between clients
void *messaging(void * ptr){
	char output[MAXLEN];
	char input[MAXLEN];
	char clientName[MAXLEN];
	int index = numUsers - 1;
	uint32_t connfd = (uint32_t) ptr;
	srand(getpid() + time(NULL) + getuid());

	//send welcome message to client
	read(connfd, usernames[index], MAXLEN);
	usernames[index][strlen(usernames[index]) - 1] = NULL;
	sprintf(output, "Welcome, %s!\n", usernames[index]);
	write(connfd, output, strlen(output));
	
	for(int i = 0; i < strlen(usernames[index]); i++){
		clientName[i] = usernames[index][i];
	}

	//read from and write to client
	while(1){
		read(connfd, input, MAXLEN);

		if(strstr(input, "list")){
			//send client the current list of users
			char list[MAXLEN * (MAXUSERS + 1) + 1];
			memset(list, '\0', sizeof(list));
			
			for(int i = 0; i < numUsers; i++){
				if(i == 0){
					sprintf(list, "%s", usernames[i]);
				} else {
					sprintf(list, "%s %s", list, usernames[i]);
				}
			}

			sprintf(list, "%s\n", list);
			write(connfd, list, sizeof(list));
		} else if(strstr(input, "close")){
			//Disconnect client from the server
			int i;

			for(i = 0; i < numUsers; i++){
				if(clientfds[i] == connfd){
					break;
				}
			}

			for(int j = i; j < numUsers; j++){
				clientfds[j] = clientfds[j + 1];
				strcpy(usernames[j], usernames[j+1]);
			}
			
			memset(usernames[numUsers - 1], '\0', MAXLEN);

			numUsers--;
			break;
		} else {
			//send client's message to another client
			char* args[3]; 
			
			if(strstr(input, "send")){
				//send message to user specified by client
				args[0] = strtok(input, " \n");
				args[1] = strtok(NULL, " \n");
				args[2] = strtok(NULL, "\n");

				bool found = false;

				for(int i = 0; i < numUsers; i++){
					//if username is found, send the message
					if(strcmp(args[1], usernames[i]) == 0){
						found = true;
						
						sprintf(output, "%s says %s\n", clientName, args[2]);
						write(clientfds[i], output, strlen(output));
						sprintf(output, "Message sent!\n");
						write(connfd, output, strlen(output));
						break;
					}
				}

				if(found == false){
					sprintf(output, "Sorry, %s has not joined yet.\n", args[1]);
					write(connfd, output, strlen(output));
				}
			} else if(strstr(input, "random")){
				//send message to random currently connected user
				args[0] = strtok(input, " \n");
				args[1] = strtok(NULL, "\n");

				sprintf(output, "%s says %s\n", clientName, args[1]);
				write(clientfds[rand() % numUsers], output, strlen(output));
				sprintf(output, "Message sent!\n");
				write(connfd, output, strlen(output));
			} else if(strstr(input, "broadcast")){
				//send message to all currently connected users
				args[0] = strtok(input, " \n");
				args[1] = strtok(NULL, "\n");

				sprintf(output, "%s says %s\n", clientName, args[1]);

				for(int i = 0; i < numUsers; i++){
					if(clientfds[i] != connfd){
						write(clientfds[i], output, strlen(output));
					}
				}

				sprintf(output, "Message sent!\n");
				write(connfd, output, strlen(output));
			}
		}
	}

	sprintf(output, "Goodbye, %s!\n", clientName);
	write(connfd, output, strlen(output));
	printf("Disconnected client %d.\n", index + 1);
	close(connfd);
}

//Main function to setup server
int main(){
	//set up sockets for client-server interaction
	int listenfd = 0, connfd = 0;
	struct sockaddr_in serv_addr;

	char buffer[1025];
	time_t ticks;
	
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		pexit("socket() error.");
	}

	memset(&serv_addr, '0', sizeof(serv_addr));
	memset(buffer, '0', sizeof(buffer));

	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

	int port = 4999;
	do{
		port++;
		serv_addr.sin_port = htons(port);
	} while(bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0);
	printf("bind() succeeds for port #%d\n", port);

	if(listen(listenfd, 10) < 0){
		pexit("listen() error.");
	}

	printf("Connect to <server IP> %d to use IM!\n", port);
	
	//loop to keep reading input from clients
	char line[MAXLEN];
	pthread_t thread1;

	while(1){
		//take requests from clients and create a new thread to handle them
		connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
		clientfds[numUsers] = connfd;
		numUsers++;
		count++;
		fprintf(stderr, "connected to client %d. \n", count);
		pthread_create(&thread1, NULL, messaging, (void *) connfd);
	}
}
