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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <signal.h>
#include <pthread.h>
#include <strings.h>

#define MAXWORDS 100000
#define MAXLEN 1000

char *words[MAXWORDS];
int numWords = 0;
int clientfds[1024];
int clientIndex = 0;

void pexit(char *errmsg){
	fprintf(stderr, "%s\n", errmsg);
	exit(1);
}

void playGame(void *ptr){
	uint32_t connfd = (uint32_t) ptr;

	//set up the hidden word to display to client
	char *word = words[rand() % numWords];
	int n = strlen(word);
	char display[n];
	strcpy(display, word);

	for(int i = 0; i < n; i++){
		display[i] = '*';
	}

	//play hangman with the client
	int unexposed = n;
	char guess[100];
	int wrongGuesses = 0;
	char prompt[MAXLEN];

	while(unexposed > 0){
		//write to client
		sprintf(prompt, "(Guess) Enter a letter in word %s > ", display);
		write(connfd, prompt, strlen(prompt));
		//read from client
		read(connfd, guess, 1);

		bool found = false;
		for(int i = 0; i < n; i++){
			if(guess[0] == word[i]){
				found = true;
				if(guess[0] == display[i]){
					//write to client guess is already in word
					sprintf(prompt, "\t%c is already in the word.\n", guess[0]);
					write(connfd, prompt, strlen(prompt));
					break;
				} else {
					//correct guess
					display[i] = guess[0];
					unexposed--;
				}
			}
		}

		if(!found){
			//write to client guess not in word
			sprintf(prompt, "\tNot in the word: %c\n", guess[0]);
			write(connfd, prompt, strlen(prompt));
			wrongGuesses++;
		}
	}

	//write to client "The word is...\n"
	//write "You missed... times.\n"
	sprintf(prompt, "The word is %s. You missed %d times.", word, wrongGuesses);
	write(connfd, prompt, strlen(prompt));
}

int main(int argc, char *argv[]){
	//open dictionary.txt for reading
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

	//code to create threads when a client connects to server
	pthread_t thread1;
	while(1){
		connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
		clientfds[clientIndex] = connfd;
		clientIndex++;
		fprintf(stderr, "connected to client %d.\n", clientIndex);
		pthread_create(&thread1, NULL, playGame, (void *) connfd);
	}
}
