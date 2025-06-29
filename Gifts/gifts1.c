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

int main(int argc, char *argv[]){
	//code here! use the following code just as reference
	char *ptr;

	if(strcmp(argv[1], "new") == 0){
		FILE *fp = fopen("gifts1.txt", "w");

		for(int i = 2; i < argc; i++){
			if((i % 2) != 0){
				balances[numPlayers] = strtod(argv[i], &ptr);

				fprintf(fp, "%10s %6.2lf\n", names[numPlayers], balances[numPlayers]);
				numPlayers++;
			} else {
				for(int j = 0; j < strlen(argv[i]); j++){
					names[numPlayers][j] = argv[i][j];
				}
			}

		}

		fclose(fp);
	} else {
		FILE *finp = fopen("gifts1.txt", "r");

		for(int i = 0; i < MAXPLAYERS; i++){
			if(fscanf(finp, "%s%lf", names[i], &balances[i]) < 2){
				break;
			}

			numPlayers++;
		}

		fclose(finp);
		balances[findIndex(argv[1])] -= strtod(argv[2], &ptr);
		double gift = strtod(argv[2], &ptr) / (argc - 3);
		
		FILE *fp = fopen("gifts1.txt", "w");

		for(int i = 0; i < numPlayers; i++){
			for(int j = 3; j < argc; j++){	
				if(strcmp(names[i], argv[j]) == 0){
					balances[i] += gift;
				}
			}
			
			fprintf(fp, "%10s %6.2lf\n", names[i], balances[i]);
		}

		fclose(fp);
	}

	for(int i = 0; i < numPlayers; i++){
		printf("%10s: %6.2lf\n", names[i], balances[i]);
	}

	/*
	FILE *filep = fopen("gifts1.txt", "w");
	int n = 5; //let us assume players
	fprintf(filep, "%s %lf\n", names[0], balances[0]);
	fclose(filep);

	//when reading text data, we may not know how many players
	//you can read the following in a loop
	if (fscanf("%s%lf", names[i], &balances[i]) < 2)
		break;
	*/
}
