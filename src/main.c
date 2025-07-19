#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <stdbool.h>


#define MAX_SIZE 2056

#define PING "+PONG\r\n"

bool isSymbol(char c){

	return c == '*' || c == '$'; 


}

void addEncoding(char* parsed , int i){

	parsed[i] = '\r';

	parsed[i + 1] = '\n'; 



}

int convertInt(char *c){

	int res = 0; 

	for(int i = 1 ; i < strlen(c); i++){

		res = res * 10 + (c[i] - '0');

	}

	return res; 

}


	
	

void executeCommand(char *arr[], int len , int fd ){

	char* command = arr[0]; 


	if(strcmp(command , "ECHO") == 0){

		
		for(int i = 1 ; i < len; i++){

			int len = strlen(arr[i]);

			char* parsed = (char*)malloc(strlen(arr[i]) + sizeof(len) + 5); 

			sprintf(parsed,"$%d\\r\\n%s\\r\\n", len, arr[i]);


			send(fd , parsed , strlen(parsed), 0);  

			free(parsed); 

		}


	}

	else{

		send(fd , PING , strlen(PING), 0);
	}

}
void handleCommand(char buf[] , int fd){

	char *curr;
	curr = strtok(buf , "\\r\\n");

	int len = convertInt(curr); 

	char *arr[len]; 

	int i = 0; 
	
	while((curr = strtok(NULL, "\\r\\n")) != NULL){

		if(!isSymbol(curr[0])){

			arr[i] = curr; 
			i++; 

		}



	}

	executeCommand(arr , len , fd); 



}




int main() {
	// Disable output buffering
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	
	// You can use print statements as follows for debugging, they'll be visible when running tests.
	printf("Logs from your program will appear here!\n");

	// Uncomment this block to pass the first stage
	
	int server_fd, client_addr_len;
	struct sockaddr_in client_addr;
	
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}
	
	// Since the tester restarts your program quite often, setting SO_REUSEADDR
	// ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEADDR failed: %s \n", strerror(errno));
		return 1;
	}
	
	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
									 .sin_port = htons(6379),
									 .sin_addr = { htonl(INADDR_ANY) },
									};
	
	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
	}
	
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}
	
	printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);
	
	while(1){
	

	int cl = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t*)&client_addr_len);

	char buf[MAX_SIZE];


	if(fork() == 0){


	int len = 0 ; 
	while((len = recv(cl , buf , MAX_SIZE , 0)) != 0){



		handleCommand(buf, cl);


	

	


}

	close(cl);
	exit(0);

}}









	
	close(server_fd);

	return 0;
}
