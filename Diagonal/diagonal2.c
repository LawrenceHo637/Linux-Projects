#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define LINESIZE 16

//use one command line argument
int main(int argc, char *argv[]) {
	if (argc < 2) {
		printf("Usage: diagonal <textstring>\n");
		return -1;
	}
	
	//create a file so that 16 rows of empty will appear with od -c command
	int fd = open("diagonal2.out", O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
	char space = ' ';
	for(int j = 1; j < argc; j++){
		for(int line = 0; line < LINESIZE; line++){
			for(int column = 0; column < LINESIZE; column++){
				write(fd, &space, 1);
			}
		}

	//Each line of od outputs 16 characters 
	//So, to make the output diagonal, we will use 0, 17, 34, ....
		int n = strlen(argv[j]);

		if((j % 2) != 0){	
			lseek(fd, LINESIZE * 16 * (j - 1), SEEK_SET);

			for(int i = 0; i < n; i++){
				write(fd, &argv[j][i], 1);
				lseek(fd, LINESIZE, SEEK_CUR);
			}
		} else {
			lseek(fd, (LINESIZE * 16 * (j - 1)) + 15, SEEK_SET);

			for(int i = 0; i < n; i++){
				write(fd, &argv[j][i], 1);
				lseek(fd, LINESIZE - 2, SEEK_CUR);
			}
		}

		lseek(fd, 0, SEEK_END);
	}
	close(fd);
	puts("diagonal2.out has been created. Use od -c diagonal2.out to see the contents.");
}
