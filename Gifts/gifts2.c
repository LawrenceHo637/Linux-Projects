#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

//let us assume that there will not be more than 100 players
#define MAXPLAYERS 100
//let us assume that max length for any name is 100 characters
#define MAXLEN 100

//arrays to store the player names and balances
char names[MAXPLAYERS][MAXLEN];
double balances[MAXPLAYERS];
int numPlayers = 0; //set when actual player data is loaded

//search the player names array and return the index if specified name is found
//return -1 otherwise.
int findIndex(char *name) {
        for(int i=0; i<numPlayers; i++)
            if(strcmp(name, names[i]) == 0)
               return i;

        return -1;
}

// use binary data file gifts2.dat to read and store the results.

int main(int argc, char *argv[]) {
	//code here!
	char *ptr;
	int n;
	double balance;

	if(strcmp(argv[1], "new") == 0){
		int fd = open("gifts2.dat", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
		
		for(int i = 2; i < argc; i++){
			if((i % 2) != 0){
				balances[numPlayers] = strtod(argv[i], &ptr);
				balance = balances[numPlayers];

				write(fd, &n, sizeof(int));
				write(fd, names[numPlayers], n);
				write(fd, &balance, sizeof(double));
				numPlayers++;
			} else {
				n = strlen(argv[i]) + 1;
				
				for(int j = 0; j < strlen(argv[i]); j++){
					names[numPlayers][j] = argv[i][j];
				}
			}
		}

		close(fd);
	} else {
		int fd = open("gifts2.dat", O_RDONLY);

		while(read(fd, &n, sizeof(int)) != 0){
			read(fd, names[numPlayers], n);
			read(fd, &balance, sizeof(double));

			balances[numPlayers] = balance;

			numPlayers++;
		}

		close(fd);
		balances[findIndex(argv[1])] -= strtod(argv[2], &ptr);
		double gift = strtod(argv[2], &ptr) / (argc - 3);
		fd = open("gifts2.dat", O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);

		for(int i = 0; i < numPlayers; i++){
			for(int j = 3; j < argc; j++){
				if(strcmp(names[i], argv[j]) == 0){
					balances[i] += gift;
				}
			}

			n = strlen(names[i]) + 1;
			balance = balances[i];

			write(fd, &n, sizeof(int));
			write(fd, names[i], n);
			write(fd, &balance, sizeof(double));
		}

		close(fd);
	}

	for(int i = 0; i < numPlayers; i++){
		printf("%10s: %6.2lf\n", names[i], balances[i]);
	}

	//following line makes sense only for writing to file
	//int fd = open("gifts2.dat", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
	//if the command is not "new",
	//int fd = open("gifts2.dat", O_RDONLY);

	//how to write and read individual name?
	//simplest approach is to write the name length first, then the actual content
	//to make reading the name from file easier.
	//write(fd, balances, sizeof(double));
	//close(fd);
}
