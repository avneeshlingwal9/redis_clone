

#include <netinet/in.h>
#include <netinet/ip.h>

#include <errno.h>


#include <stdbool.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include "parser.h"
#include <sys/wait.h>

#define MAX_CONNECTIONS 32

char* dir; 

char* dbfilename; 

char* filePath; 

#define MAX_SIZE 2056
int createDatabase(){

	if(!fileExists(filePath)){

		printf("Database Do not Exist.\n"); 


			return 1; 

		}

		FILE *file = fopen(filePath, "rb");

		int curr ; 
		// Read till Database Section.
		while((curr = fgetc(file)) != EOF && curr != 0xFE){}; 

		// Skip two bytes to find length of hash table. 

		fseek(file , 2 , SEEK_CUR);

		// Parse Len.

		int hashlen = parseLengthEncoding(file); 

		int expiryKeys = parseLengthEncoding(file);

		for(int i = 0 ; i < hashlen ; i++){

			curr = fgetc(file);

			if(curr == 0x00){
				// No expiry. 

				// Read string length. 

				char* key = decodeString(file); 
/* 				int lenKey = parseLengthEncoding(&file); 

				char* key = (char*)malloc(lenKey + 1); 

				int start = 0 ; 

				while(start < lenKey){

					key[start] = fgetc(file);
					start++;
				}

				key[lenKey] = '\0';  */
				char* value = decodeString(file); 

				double expiry = -1; 

				setValue(key, value , expiry); 

				free(key);
				free(value);

				
			}

			else if(curr == 0xFC){
				// Expiry in milliseconds. 

				uint64_t millisecond = decodeMilliSeconds(file);

				double expiry = millisecond; 

				curr = fgetc(file);

				if(curr == 0x00){
					// String type; 

					char* key = decodeString(file);
					char* value = decodeString(file);


					setValue(key , value, expiry);

					free(key);
					free(value);

				}



			}

			else if(curr == 0xFD){

				u_int32_t expiry = decodeSeconds(file);

				curr = fgetc(file);

				if(curr == 0x00){

					char* key = decodeString(file);

					char* value = decodeString(file);

					setValue(key, value, expiry); 

					free(key);
					free(value);


				}

			}


		}


		printf("Database imported successfully.\n"); 

		fclose(file);

		return 0; 




}

int makeNonBlocking(int fd){

	int flags = fcntl(fd , F_GETFL, 0);
	return fcntl(fd , F_SETFL , flags | O_NONBLOCK); 

}

int execute(int fd , char** arguments , int numArgs){

	Commands command = parseCommand(arguments[0]); 

	free(arguments[0]); 

	createDatabase();



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

	else if(command == GET){



		char* key = arguments[1]; 

		char* value = getValue(key); 

		free(key); 

		if(value == NULL){

			send(fd , RESP_NULL , strlen(RESP_NULL) , 0); 

		}

		else{

			char* tosend = encodeStr(value);
			
			free(value); 

			send(fd , tosend , strlen(tosend) , 0); 

			free(tosend); 


		}



	}

	else if(command == SET){


		char* key = arguments[1];
		char* value = arguments[2]; 

		int expiry = -1; 

		if(numArgs > 3){

			Options option = parseOption(arguments[3]);

			free(arguments[3]);

			expiry = atoi(arguments[4]);

		}

		setValue(key , value ,expiry);

		free(key);
		free(value);

		send(fd , RESP_OK , strlen(RESP_OK), 0); 






	}
	else if(command == CONFIG){

		char* opt = arguments[1];
		command = parseCommand(opt);
		free(opt);
		if(command == GET){

			char* key = arguments[2]; 
			char* toSend; 

			if(strcmp(key, "dir") == 0){

				char** array = (char**)malloc(sizeof(char*) * 2);
				
				array[0] = strdup(key); 
				array[1] = strdup(dir); 
				
				toSend = encodeArray(array , 2); 

				for(int i = 0 ; i < 2 ; i++){

					free(array[i]);

				}

				free(array);


			}

			else if(strcmp(key,"dbfilename") == 0){

				char** array = (char**)malloc(sizeof(char*)* 2); 

				array[0] = strdup(key); 

				array[1] = strdup(dbfilename);


				toSend = encodeArray(array , 2);

				for(int i = 0 ; i < 2 ; i++){

					free(array[i]);
				}

				free(array); 
			}

			if(toSend == NULL){
				printf("Error in encoding array.\n");
			}
			else{

				send(fd , toSend , strlen(toSend), 0);

			}

			free(toSend);
			free(key); 

			



		}
		else{

			printf("Error in executing config.\n"); 

			

		}

	}

	
	else if(command == KEYS){

		char** keys ; 

		int numKey = getKeys(&keys); 

		if(keys == NULL || numKey == 0){

			printf("No keys found.\n");

		}
		else{


		char* toSend = encodeArray(keys,numKey );

		for(int i = 0 ; i < numKey ; i++){



			free(keys[i]);
		}

		send(fd , toSend , strlen(toSend), 0); 
			
		free(toSend);
		free(keys); 
	
	}



		
		
		






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







int main(int argc , char* argv[]) {


	// Disable output buffering
	setbuf(stdout, NULL);
	setbuf(stderr, NULL);
	if(argc == 5){

		dir = strdup(argv[2]);

		dbfilename = strdup(argv[4]); 
		// You can use print statements as follows for debugging, they'll be visible when running tests.
		printf("Logs from your program will appear here!\n");

		// Uncomment this block to pass the first stage

		filePath = (char*)malloc(strlen(dir) + strlen(dbfilename) + 2); 

		sprintf(filePath , "%s/%s", dir , dbfilename);

	}

/* 	if(fork() == 0){

		createDatabase(); 

	} */
	
	
/* 	else{

		wait(0); */


	
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

//}





	freeDictionary();

	free(dir);
	free(dbfilename); 

	free(filePath);



	

	


	return 0;
}
