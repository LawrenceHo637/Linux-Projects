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
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/stat.h>

int main(int argc, char *argv[]){
	//set up socket
	int sockfd = 0, n = 0;
	char recvBuff[1024];
	struct sockaddr_in serv_addr;

	if(argc != 3){
		printf("\n Usage: %s <IP of server> <port #> \n", argv[0]);
		return 1;
	}

	memset(recvBuff, '0', sizeof(recvBuff));
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) <0){
		printf("\n Error: Could not create socket \n");
		return 1;
	}

	memset(&serv_addr, '0', sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	int port = atoi (argv[2]);
	serv_addr.sin_port = htons(port);

	if(inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) <= 0){
		printf("\n inet_pton error occured\n");
		return 1;
	}

	if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0){
		printf("\n Error : Connect Failed \n");
		return 1;
	}

	//Code to process server-client interaction
	while(1){
		n = read(sockfd, recvBuff, sizeof(recvBuff) - 1);
		recvBuff[n] = 0;

		if(strstr(recvBuff, "Enter a")){
			printf("%s", recvBuff);
			char response[100];
			scanf("%s", response);
			
			//send one char to server
			write(sockfd, response, strlen(response));
		} else {
			printf("%s", recvBuff);

			if(strstr(recvBuff, "The word is")){
				break;
			}
		}
	}

	return 0;
}
