

#include <netinet/in.h>
#include <netinet/ip.h>

#include <errno.h>


#include <stdbool.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include "parser.h"

#define MAX_CONNECTIONS 32


#define MAX_SIZE 2056

int makeNonBlocking(int fd){

	int flags = fcntl(fd , F_GETFL, 0);
	return fcntl(fd , F_SETFL , flags | O_NONBLOCK); 

}

int execute(int fd , char** arguments , int numArgs){

	Commands command = parseCommand(arguments[0]); 

	free(arguments[0]); 



	if(command == PING){

			send(fd , RESP_PONG , strlen(RESP_PONG) , 0); 

		}

	else if(command == ECHO){

			
			char* arg = arguments[1]; 

			int digits = snprintf(NULL , 0 , "%d", (int)strlen(arg)); 



			char* toSend = (char*)malloc(strlen(arg) + digits + 6); // Extra for '\0'



			sprintf(toSend , "$%d\r\n%s\r\n" , (int)strlen(arg), arg);


			

			send(fd , toSend , strlen(toSend), 0); 

			free(arg);
			free(toSend); 
			
		}


		return 1; 
}

int handleConnection(int fd){


	char* buf = (char*)malloc(MAX_SIZE); 
	char* input = buf; 


	int bytesRead = read(fd , buf , MAX_SIZE);
	if(bytesRead == 0){
		return 0; 
	}
	
	char* end = buf + bytesRead; 

	int numArgs = parseLen(&input); 

	if(numArgs <= 0){
		printf("No arguments found.\n"); 
		return 0;
	}

	char** arguments = (char**)malloc(numArgs* sizeof(char*)); 

	if(parseArray(&input , end , &arguments , numArgs) == true){

		execute(fd , arguments , numArgs); 

	}

	else{

		printf("Incomplete parsing.\n"); 

	}

	free(arguments);
	free(buf); 

	return 0; 







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

	int epoll_fd = epoll_create1(0) , nfds;

	struct epoll_event ev, events[MAX_CONNECTIONS];

	makeNonBlocking(server_fd);

	ev.events = EPOLLIN;
	ev.data.fd = server_fd; 

	if(epoll_ctl(epoll_fd , EPOLL_CTL_ADD , server_fd , &ev) != 0){

		perror("Error occured while adding file descriptor.\n");
		return 3; 

	} 
	
	while(1){
	
		nfds = epoll_wait(epoll_fd , events , MAX_CONNECTIONS , -1); 


		for(int i = 0 ; i < nfds ; i++){

			if(events[i].data.fd == server_fd){

				// Add a new connection.

				int cl = accept(server_fd, (struct sockaddr *) &client_addr, (socklen_t*)&client_addr_len);

				makeNonBlocking(cl);

				printf("Client Connected.\n"); 

				ev.data.fd = cl; 
				ev.events = EPOLLIN;

				// Adding fd to monitor list. 

				epoll_ctl(epoll_fd , EPOLL_CTL_ADD , cl , &ev); 

			}
			else{

				// Something is available for reading from previous connections. 

				handleConnection(events[i].data.fd);




			}


		}

	






}









	
	close(server_fd);

	return 0;
}
