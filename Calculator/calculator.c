#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

#define MAXLEN 1000

char operators[100];
int fds[201][2];
int operatorCount=0;
int numPipes=0;

void child(int index) {
	//CODE HERE!
	dup2(fds[2 * index][0], 0);
	dup2(fds[(2 * index) + 1][0], 3);
	dup2(fds[(2 * index) + 2][1], 1);
	
	for(int i = 0; i < numPipes; i++){
		close(fds[i][0]);
		close(fds[i][1]);
	}

	if(operators[index] == '+'){
		execl("./add", "add", NULL);
	} else if(operators[index] == '-'){
		execl("./subtract", "subtract", NULL);
	} else if(operators[index] == '*'){
		execl("./multiply", "multiply", NULL);
	} else {
		execl("./divide", "divide", NULL);
	}
	
	fprintf(stderr, "...\n");
	//reconfigure appropriate pipes (plumbing) to fds 0, 3 and 1
	//close all pipes! IF missed, you will end up with lot of orphans! 
	// operators[index] +         -        *            /
	// execl            add       subtract mult         divide
	//printf() is bad idea! --> fprintf(stderr, "...\n", 
}

int main(int argc, char *argv[]) {
	char line[MAXLEN], *temp;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
		exit(1);
	}

	FILE *dataFile = fopen(argv[1], "r");
	//read the first line - it contains the configuration
	fgets(line, MAXLEN, dataFile); 

	// sample content for line: a + b - c
	strtok(line, " \n"); //skip the symbol representing variable/parameter
	while (temp = strtok(NULL, " \n")) {
		operators[operatorCount] = temp[0];
		//printf("operator: %c\n", operators[operatorCount]);
		operatorCount++;
		strtok(NULL, " \n"); //skip the symbol representing variable/parameter
	}

	//create the necessary # of pipes
	numPipes = operatorCount * 2 + 1;

	//CODE HERE!
	//loop: create that many pipes (numPipes) -- all pipes are created!
	for(int i = 0; i < numPipes; i++){
		pipe(fds[i]);
	}

	//loop: create that many children (operatorCount)
	for(int i = 0; i < operatorCount; i++){
		if(!fork()){
			child(i);
		}
	}

	//proceed to read data from the file and keep pumping them into pipes and read result 
	//while (fscanf(dataFile, "%d", &value) > 0)
	//  write it to first pipe
	//	use loop to read remaining data in that line & keep puming to pipes
	//	read the final result from the final pipe & print
	int value;
	while(fscanf(dataFile, "%d", &value) > 0){
		int i = 0;
		write(fds[i][1], &value, sizeof(int));
		i++;
		
		for(int j = 0; j < operatorCount; j++){
			fscanf(dataFile, "%d", &value);
			write(fds[i][1], &value, sizeof(int));
			i += 2;
		}

		read(fds[i - 1][0], &value, sizeof(int));
		printf("%d\n", value);
	}
}
