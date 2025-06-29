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

#define MAXUSERS 1000
#define MAXLEN 1000

int numUsers = 0;
char usernames[MAXUSERS][MAXLEN];
FILE* clientfps[MAXUSERS];

/*
 *server-child thread:
 	get "send userName msg"
	lookup userName in arr
	found at index
	fprintf(clientfps[index], ...);

 *main()
 	open wellKnownFifo
	while
 		get clientFileName userName
		copy username into usernames[numClients] (strcpy)
		clientfps[numClients] = fopen(clientfilename, "w");
		create child thread (numClients);
		numClients++;
 */

void *messaging(void * num){
	char input[MAXLEN];
	int n = numUsers - 1;
	
	//create serverfifo for child process and send to client
	char serverfifo[MAXLEN];
	sprintf(serverfifo, "/tmp/%s-%d", getenv("USER"), n);
	mkfifo(serverfifo, 0600);
	chmod(serverfifo, 0622);

	fprintf(clientfps[n], "%s\n", serverfifo);
	fflush(clientfps[n]);

	FILE *serverfp = fopen(serverfifo, "r");
	if(!serverfp){
		printf("FIFO %s cannot be opened for reading.\n", serverfifo);
		exit(3);
	}

	//send welcome message to client
	fprintf(clientfps[n], "Welcome!\n");
	fflush(clientfps[n]);

	//read from and write to client
	while(1){
		fgets(input, MAXLEN, serverfp);

		if(strstr(input, "list")){
			//send client the current list of users
			char list[MAXLEN * (MAXUSERS + 1)];

			for(int i = 0; i < numUsers; i++){
				if(i == 0){
					sprintf(list, "%s", usernames[i]);
				} else {
					sprintf(list, "%s %s", list, usernames[i]);
				}
			}

			fprintf(clientfps[n], "%s\n", list);
			fflush(clientfps[n]);
		} else if(strstr(input, "send")){
			//send client's message to another client
			char* args[3]; 
			args[0] = strtok(input, " \n");
			args[1] = strtok(NULL, " \n");
			args[2] = strtok(NULL, "\n");
			bool found = false;

			for(int i = 0; i < numUsers; i++){
				//if username is found, send the message
				if(strcmp(args[1], usernames[i]) == 0){
					found = true;
					fprintf(clientfps[i], "%s says %s\n", usernames[n], args[2]);
					fflush(clientfps[i]);
					fprintf(clientfps[n], "Message sent!\n");
					fflush(clientfps[n]);
					break;
				}
			}

			if(found == false){
				fprintf(clientfps[n], "Sorry, %s has not joined yet.\n", args[1]);
				fflush(clientfps[n]);
			}
		}
	}
}

int main(){
	//create named pipe to read from client
	char publicfifo[MAXLEN];
	sprintf(publicfifo, "/tmp/%s-%d", getenv("USER"), getpid());
	mkfifo(publicfifo, 0600);
	chmod(publicfifo, 0622);
	printf("Connect to %s to use IM!\n", publicfifo);
	
	//loop to keep reading input from clients
	char line[MAXLEN];

	while(1){
		FILE *publicfp = fopen(publicfifo, "r");
		if(!publicfp){
			printf("FIFO %s cannot be opened for reading.\n", publicfifo);
			exit(2);
		}

		//take requests from clients and fork to handle them
		char clientfifo[MAXLEN];
		if(fgets(clientfifo, MAXLEN, publicfp) != NULL){
			char *cptr = strchr(clientfifo, '\n');
			if(cptr){
				*cptr = '\0';
			}
			
			clientfps[numUsers] = fopen(clientfifo, "w");
			fgets(line, MAXLEN, publicfp);
			line[strlen(line) - 1] = '\0';

			strcpy(usernames[numUsers], line);
			printf("%s joined!\n", usernames[numUsers]);

			pthread_t thread;
			pthread_create(&thread, NULL, messaging, (void *) numUsers);
			numUsers++;
		}

		fclose(publicfp);
	}
}
