/* make up unique pipename for client
 * send it to the server (public pipe)
 * get another unique pipename from the server
 * repeat
 * 	read a line including starred word from the client pipe
 *  display that line to the user 
 *  check whether game is over?
 *  get the user's guess letter
 *  send to the server using server pipe
 */

// custom client for this specific server
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>

#define MAXLEN 1000

int main(int argc, char *argv[]){
	if(argc != 2){
		puts("Usage: qclient <server-fifo-name>");
		exit(1);
	}

	//setup client fifo
	char clientfifo[MAXLEN];
	sprintf(clientfifo, "/tmp/%s-%d", getenv("USER"), getpid());
	mkfifo(clientfifo, 0600);
	chmod(clientfifo, 0622);

	//open public fifo  and write client fifo to it
	FILE *fp = fopen(argv[1], "w");
	fprintf(fp, "%s\n", clientfifo);
	fflush(fp);
	fclose(fp);

	//open clientfp for reading from server
	FILE *clientfp = fopen(clientfifo, "r");
	char serverfifo[MAXLEN];
	fscanf(clientfp, "%s", serverfifo);
	char line[MAXLEN];
	fgets(line, MAXLEN, clientfp); //gets rid of \n in serverfifo

	//open serverfp to read and write to server
	FILE *serverfp = fopen(serverfifo, "w");
	
	while (1){
		//read a line from server
		//if read fails, break;
		// fgets() returns NULL when it fails
		if(!fgets(line, MAXLEN, clientfp)){
			break;
		}

		if(strstr(line, "Enter a")){ // strstr(line, "Enter a")
			char *cptr = strchr(line, '\n');
			if(cptr){
				*cptr = '\0';
			}

			printf("%s", line);
			char response[100];
			scanf("%s", response);
		
			//send one char to server
			fprintf(serverfp, "%c", response[0]);
			fflush(serverfp);
		} else {
			printf("%s", line);
			
			if(strstr(line, "The word is")){
				break;
			}
		}
	}

	fclose(clientfp);
	unlink(clientfifo);
}
