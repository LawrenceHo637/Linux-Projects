#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/wait.h>

//read all the quotes from quotes.txt
//when client asks for a motivational quote, select one randomly and send it out.

#define MAXQUOTES 10000
#define MAXLEN 1000

char *quotes[MAXQUOTES];
int numQuotes = 0;
char *commands[MAXLEN];
int numCommands = 0;
int pipes[201][2];
int numPipes = 0;

//runs a simple command
//cmdname arg1 arg2 arg3 ...
void runCommand(char *command) {
	//split and assemble the arguments and invoke execvp()
	//use strtok(..)
	char *args[MAXLEN];
	int argc = 0;

	args[argc] = strtok(command, " \n");
	while(args[argc]){
		args[++argc] = strtok(NULL, " \n");
	}

	execvp(args[0], args);
	fprintf(stderr, "%s cannot be run.\n", args[0]);
	exit(1);
}

//cmd0 | cmd1 | cmd2 | cmd3 | cmd4 ....
void child(int i) {
	//rewire pipes to 0 and 1 
	//do NOT rewire 0 for the first command - leave it as std input
	//do NOT rewrite 1 for the last command - leave it as std output
	//close unnecessary pipes - for loop?
	//run ith command
	if(i == 0){
		dup2(pipes[i + 1][1], 1);
	} else if(i == (numCommands - 1)){
		dup2(pipes[(2 * i) - i][0], 0);
	} else {
		dup2(pipes[(2 * i) - i][0], 0);
		dup2(pipes[2 * i - (i - 1)][1], 1);
	}

	for(int i = 0; i < numPipes; i++){
		close(pipes[i][0]);
		close(pipes[i][1]);
	}

	runCommand(commands[i]);
}

void processLine(char *line) {
	char *pipePtr = strchr(line, '|'); //does this command have | chars?
	char *equalPtr = strchr(line, '='); //does this command have =?
	if (pipePtr) { //not NULL - cmd1 | cmd2 | cmd3 ....
		//command has several sub-commands connected with pipes
		//setup commands array
		//setup pipes array
		int index = 0;

		commands[index] = strtok(line, "|");
		while(commands[index]){
			commands[++index] = strtok(NULL, "|");
		}

		numCommands = index;
		numPipes = numCommands * 2 + 1;

		//fork & create children --> invoke child(i) in a loop
		//cmd0 | cmd1 | cmd2 | cmd3 | cmd4 
		// invoke child(i) for the last command directly?
		for(int i = 0; i < numPipes; i++){
			pipe(pipes[i]);
		}

		for(int i = 0; i < numCommands; i++){
			if(i == (numCommands - 1)){
				child(numCommands - 1);
			} else {
				if(!fork()){
					child(i);
				}
			}
		}
	} else if (equalPtr) { 
		//command has = operator, so 2 commands --> 2 processes
		char *cmd1 = strtok(line, "=");
		char *cmd2 = strtok(NULL, "\n");

		pipe(pipes[0]);
		pipe(pipes[1]);

		if(!fork()){
			//child handles command 1
			dup2(pipes[0][0], 0);
			dup2(pipes[1][1], 1);

			close(pipes[0][0]);
			close(pipes[0][1]);
			close(pipes[1][0]);
			close(pipes[1][1]);

			runCommand(cmd1);
		} else {
			//parent handles command 2
			dup2(pipes[1][0], 0);
			dup2(pipes[0][1], 1);
			
			close(pipes[0][0]);
			close(pipes[0][1]);
			close(pipes[1][0]);
			close(pipes[1][1]);

			runCommand(cmd2);
		}
    	} else{ 
		//it is a simple command, no pipe or = character
		runCommand(line);
	}

	exit(0);
}

int main() {
	// load up all the quotes from quotes.txt - look at quoteserver.c for the code
	char line[MAXLEN];
	int i = 0;
	FILE *fp = fopen("quotes.txt", "r");

	if(!fp){
		puts("quotes.txt cannot be opened for reading.");
		exit(1);
	}
	
	while(fgets(line, MAXLEN, fp)){
		quotes[i] = (char *) malloc (strlen(line) + 1);
		strcpy(quotes[i], line);
		i++;
	}

	numQuotes = i;

	// infinite loop to serve the customer
	srand(time(NULL));

	while (1) {
		//output a random quote to stderr
		fputs(quotes[rand()%numQuotes], stderr);
		fprintf(stderr, "# "); //show prompt to the user
		//get the user input -- command line
		fgets(line, 1000, stdin);

		//spawn a child for taking care of it
		if (fork() == 0) 
			processLine(line);

		//wait the child to finish the job!
		wait(NULL);
	}
}
