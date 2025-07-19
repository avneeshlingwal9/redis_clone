#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>

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

		res = res * 10 + c[i] - '0';

	}

	return res; 

}
char* parse(char *arg){

	int arglen = strlen(arg); 

	char* parsed = (char*)malloc(sizeof(char)*(arglen/10 + 1 + arglen + 5));

	int i = 0; 

	parsed[i++] = '$';

	char argcopy[arglen/10 + 1]; 

	sprintf(argcopy,"%d", arglen);	

	
	int j = 0; 
	int argcopylen = strlen(argcopy);
	
	while(j < argcopylen){

		parsed[i++] = argcopy[j++]; 


	}

	j = 0; 

	addEncoding(parsed , i); 

	i+= 2 ; 

	for(int k = 0 ; k < arglen; k++){

		parsed[i++] = arg[k];

	}

	addEncoding(parsed, i); 



	return parsed;


	



}
void executeCommand(char *arr[], int len , int fd ){

	char* command = arr[0]; 


	if(strcmp(command , "ECHO") == 0){

		
		for(int i = 1 ; i < len; i++){

			char* parsed = parse(arr[i]); 

			printf("%s \n", parsed);

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
	curr = strtok(buf , "\r\n");

	int len = convertInt(curr); 

	char *arr[len]; 

	int i = 0; 
	
	while((curr = strtok(NULL, "\r\n")) != NULL){

		if(!isSymbol(curr[0])){


			arr[i] = curr; 
			i++; 

		}



	}

	executeCommand(arr , len , fd); 



}