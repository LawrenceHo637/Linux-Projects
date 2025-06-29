/*
 * read dictionary file to array of words & get ready to play the hangman!
if you are using fgets() to read the word
	  cptr = strchr(line, '\n');
	  if (cptr)
	  	*cptr = '\0';
 However, since we are dealing with words, you can use fscanf(...,"%s", ...) instead!

 * wait for a request to come in (client specifies unique pipename)
 * select a random word using rand()
 * fork() to create a child to handle this client! (dedicated server process for that client)
 * fork() enables games to proceed in parallel. Parent returns to wait for new client requests
 * respond with another unique server-side pipename 
 *
 * send a bunch of stars (indicating the word length)
 * for each guess the client sends in, respond with a message 
 * and send updated display word.
 *
 * Whenever you send strings through named pipes, 
 * terminate with '\n' and also do fflush() for that data to make it to other side without getting stuck
 * in buffers.
 *
 * open public fifo
 * while (fgets()) {
 * }
 *
 *


srand(....);

wait for a client connection
rand() to select a word
fork()
	child process:
*/
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>

#define MAXWORDS 100000
#define MAXLEN 1000

char *words[MAXWORDS];
int numWords = 0;

int playGame(char *word, char *client){
	FILE *clientfp = fopen(client, "w");
	
	//create serverfifo for child process and send to client
	char serverfifo[MAXLEN];
	sprintf(serverfifo, "/tmp/%s-%d", getenv("USER"), getpid());
	mkfifo(serverfifo, 0600);
	chmod(serverfifo, 0622);

	fprintf(clientfp, "%s\n", serverfifo);
	fflush(clientfp);

	FILE *serverfp = fopen(serverfifo, "r");
	if(!serverfp){
		printf("FIFO %s cannot be opened for reading.\n", serverfifo);
		exit(3);
	}

	//set up the hidden word to display to client
	int n = strlen(word);
	//char *display = word;
	char display[n];
	strcpy(display, word);

	for(int i = 0; i < n; i++){
		display[i] = '*';
	}

	//play hangman with the client
	int unexposed = n;
	char guess;
	int wrongGuesses = 0;

	while(unexposed > 0){
		//write to client
		fprintf(clientfp, "(Guess) Enter a letter in word %s > \n", display);
		fflush(clientfp);
		//read from client
		fscanf(serverfp, "%c", &guess);

		bool found = false;
		for(int i = 0; i < n; i++){
			if(guess == word[i]){
				found = true;
				if(guess == display[i]){
					//write to client guess is already in word
					fprintf(clientfp, "\t%c is already in the word.\n", guess);
					fflush(clientfp);
					break;
				} else {
					//correct guess
					display[i] = guess;
					unexposed--;
				}
			}
		}

		if(!found){
			//write to client guess not in word
			//fprintf(clientfp, "%c is not in the word.\n", guess);
			fprintf(clientfp, "\tNot in the word: %c\n", guess);
			fflush(clientfp);
			wrongGuesses++;
		}
	}

	//write to client "The word is...\n"
	//write "You missed... times.\n"
	fprintf(clientfp, "The word is %s. You missed %d times.\n", word, wrongGuesses);
	fflush(clientfp);
}

int main(){
	char line[MAXLEN];
	FILE *fp = fopen("dictionary.txt", "r");
	
	if(!fp){
		puts("dictionary.txt cannot be opened for reading.");
		exit(1);
	}

	//read in words from dictionary.txt
	int index = 0;
	while(fgets(line, MAXLEN, fp)){
		char *cptr = strchr(line, '\n');
		
		if(cptr){
			*cptr = NULL;
		}

		words[index] = (char *) malloc (strlen(line) + 1);
		strcpy(words[index], line);
		index++;
	}

	numWords = index;
	printf("%d words were read.\n", numWords);
	srand(getpid() + time(NULL) + getuid());

	//create named pipe to read from client
	char publicfifo[MAXLEN];
	sprintf(publicfifo, "/tmp/%s-%d", getenv("USER"), getpid());
	mkfifo(publicfifo, 0600);
	chmod(publicfifo, 0622);
	printf("Send your guesses to %s.\n", publicfifo);

	while(1){
		FILE *publicfp = fopen(publicfifo, "r");
		if(!publicfp){
			printf("FIFO %s cannot be opened for reading.\n", publicfifo);
			exit(2);
		}
		
		printf("Opened %s to read... \n", publicfifo);

		//take requests from clients and fork to handle them
		char clientfifo[MAXLEN];
		fgets(clientfifo, MAXLEN, publicfp);

		char *cptr = strchr(clientfifo, '\n');
		if(cptr){
			*cptr = '\0';
		}

		char *wrd = words[abs(rand() * rand()) % numWords];
		if(!fork()){
			playGame(wrd, clientfifo);
		}

		fclose(publicfp);
	}
}
