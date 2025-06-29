#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAXLEN 1000

int main(int argc, char *argv[]){
	if(argc != 3){
		puts("Usage: imclient <server-fifo-name> <user#>");
		exit(1);
	}

	//setup client fifo
	char clientfifo[MAXLEN];
	sprintf(clientfifo, "/tmp/%s-%d", getenv("USER"), getpid());
	mkfifo(clientfifo, 0600);
	chmod(clientfifo, 0622);

	//open public fifo and write client fifo to it
	FILE *fp = fopen(argv[1], "w");
	fprintf(fp, "%s\n", clientfifo);
	fflush(fp);
	fprintf(fp, "%s\n", argv[2]);
	fflush(fp);
	fclose(fp);

	//open clientfp for reading from server
	FILE *clientfp = fopen(clientfifo, "r");
	char serverfifo[MAXLEN];
	fscanf(clientfp, "%s", serverfifo);
	char line[MAXLEN];
	fgets(line, MAXLEN, clientfp); //gets rid of \n in serverfifo

	//open serverfp to write to server
	FILE *serverfp = fopen(serverfifo, "w");

	if(fork()){
		//parent process handles basic interaction between client and server
		while (1){
			fgets(line, MAXLEN, clientfp);
			printf("%s", line);
			
			char response[MAXLEN];
			fgets(response, MAXLEN, stdin);
			fprintf(serverfp, "%s\n", response);
			fflush(serverfp);
		}
	} else {
		//child handles receiving messages from other clients
		while(1){
			fgets(line, MAXLEN, clientfp);
			printf("%s", line);
		}
	}

	fclose(clientfp);
	unlink(argv[2]);
}
