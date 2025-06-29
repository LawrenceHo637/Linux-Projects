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

struct Player {
	char name[MAXLEN];
	double balance;
} ;

//struct is like class - we can use an array of struct (we can use like an array of objects).
struct Player players[MAXPLAYERS];
int numPlayers = 0; //set when actual player data is loaded

//search the player names array and return the index if specified name is found
//return -1 otherwise.
int findIndex(char *name) {
        for(int i=0; i<numPlayers; i++)
            if(strcmp(name, players[i].name) == 0)
               return i;

        return -1;
}

// use binary data file gifts3.dat to read and store the results.

int main(int argc, char *argv[]) {
	//code here!
	char *ptr;

	if(strcmp(argv[1], "new") == 0){
		FILE *fp = fopen("gifts3.dat", "wb");

		for(int i = 2; i < argc; i++){
			if((i % 2) != 0){
				players[numPlayers].balance = strtod(argv[i], &ptr);
				numPlayers++;
			} else {
				for(int j = 0; j < strlen(argv[i]); j++){
					players[numPlayers].name[j] = argv[i][j];
				}
			}
		}

		fwrite(players, sizeof(struct Player), numPlayers, fp);
		fclose(fp);
	} else {
		FILE *fp = fopen("gifts3.dat", "rb");
		numPlayers = fread(players, sizeof(struct Player), MAXPLAYERS, fp);
		fclose(fp);

		players[findIndex(argv[1])].balance -= strtod(argv[2], &ptr);
		double gift = strtod(argv[2], &ptr) / (argc - 3);
		
		for(int i = 0; i < numPlayers; i++){
			for(int j = 3; j < argc; j++){
				if(strcmp(players[i].name, argv[j]) == 0){
					players[i].balance += gift;
				}
			}
		}

		fp = fopen("gifts3.dat", "wb");
		fwrite(players, sizeof(struct Player), numPlayers, fp);
		fclose(fp);
	}

	for(int i = 0; i < numPlayers; i++){
		printf("%10s: %6.2lf\n", players[i].name, players[i].balance);
	}

/*
	//reading data - array of Struct - just one fread()
	FILE *filep = fopen("gifts3.dat", "rb"); //for reading
	numPlayers = fread(players, sizeof(struct Player), MAXPLAYERS, filep);
	fclose(filep);
	...
	//writing data - array of Struct - just one fwrite()
	filep = fopen("gifts3.dat", "wb"); //for writing
	fwrite(players, sizeof(struct Player), numPlayers, filep);
	fclose(filep);
*/
}
