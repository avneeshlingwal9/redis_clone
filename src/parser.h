#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdbool.h>
#include "datastructures.h"

#define RESP_PONG "+PONG\r\n"
#define RESP_OK "+OK\r\n"
#define RESP_NULL "$-1\r\n"

#define MAX_ARGS 1024
#define INFINITY 1000000000
#define RESP_NULL_ARRAY "*0\r\n"


char * parseBulkString( char ** input , int length){

	char *str = (char*)malloc(length + 1);

	if(str == NULL){

		printf("Not able to allocate memory.\n"); 

		return NULL;

	}


	strncpy(str , *input , length); 

	str[length] = '\0'; 

	(*input) += length;

	(*input) += 2 ; // Skip CRLF.

	return str; 

}

int parseLen( char **input){


	(*input)++; // Skip the initial character. 

	int length = 0 ; 

	// Atoi type function. 

	while(**input != '\r'){


		length = length * 10 + (**input - '0'); 

		(*input)++; 

		if(input == NULL){

			printf("Parsing length error.\n");

			return -1;

		}

	}

	(*input) += 2; // Skip CRLF. 

	return length;




}

bool parseArray(char** buf , char* end , char*** arguments , int numArg){




	for(int i = 0 ; i < numArg; i++){

		int len = parseLen(buf);

		if(len <= 0){
			printf("Parsing Error.\n");

			return NULL; 
		}

		if(*(buf) + len + 2  > end){

			return false;
		}

		char* arg = parseBulkString(buf , len); 

		if(arg == NULL){

			printf("Parsing Error.\n");

			return NULL; 

		}

		(*arguments)[i] = strdup(arg);

		free(arg); 

	}



	return true; 






}

Commands parseCommand(char* comm){

	if(strcasecmp(comm , "ping") == 0){

		return PING;
	}

	if(strcasecmp(comm , "echo") == 0){

		return ECHO; 

	}

	printf("Command not found: %s \n", comm);

	return UNKNOWN; 


}

