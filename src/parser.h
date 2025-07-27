
#include "algorithms.h"




char* parseBulkString( char ** input , int length){

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
	if(strcasecmp(comm ,"set") == 0){

		return SET;
	}

	if(strcasecmp(comm , "get") == 0){

		return GET; 
	}

	if(strcasecmp(comm, "config") == 0){

		return CONFIG; 
	}
	if(strcasecmp(comm, "keys") == 0){

		return KEYS;

	}

	if(strcasecmp(comm , "INFO") == 0){

		return INFO; 

	}
	if(strcasecmp(comm, "replconf") == 0){
		
		return REPLCONF;
	}

	if(strcasecmp(comm, "psync") == 0){

		return PSYNC;

	}

	printf("Command not found: %s \n", comm);

	return UNKNOWN; 


}


char* encodeStr(char *str){

    int len = strlen(str);

    int size = snprintf(NULL , 0 , "$%d\r\n%s\r\n", len , str) + 1 ; 

    char* encodedStr = (char*)malloc(size);

    snprintf(encodedStr, size  , "$%d\r\n%s\r\n", (int)strlen(str) ,str);

    return encodedStr;
    

}

Options parseOption(char* opt)
{

	if(strcasecmp(opt, "px") == 0){

		return PX;

	}
	if(strcasecmp(opt, "replication") == 0){

		return REPLICATION;

	}

	if(strcasecmp(opt , "listening-port") == 0){

		return LISTENINGPORT; 

	}

	if(strcasecmp(opt, "capa") == 0){
		return CAPA;
	}
	if(strcasecmp(opt, "psync2") == 0){
		
		return PSYNC2;
	}

	printf("Option %s not found.\n", opt); 

	return OTHER;

}


char* encodeArray(char** values , int numEl){

    // 1 for NULL terminator.
    int totalSize = snprintf(NULL , 0 , "*%d\r\n" , numEl) + 1; 

    for(int i = 0 ; i < numEl; i++){

        int len = strlen(values[i]); 
        
        totalSize += snprintf(NULL , 0 , "$%d\r\n%s\r\n", len , values[i]);


    }
    
    
    char* encodedArray = (char*)malloc(totalSize); 

    int offset = snprintf(encodedArray, totalSize, "*%d\r\n" , numEl);

    for(int i = 0 ; i < numEl; i++){

        int len = strlen(values[i]); 

        offset += snprintf(encodedArray + offset , totalSize , "$%d\r\n%s\r\n" , len , values[i]);


    }

	


    return encodedArray;


   


}

int parseLengthEncoding(FILE* file){

	int curr = fgetc(file); 

	// Get first two bits.
	// Mask first two bits.

	u_int8_t type = (curr & 0xC0) >> 6; 

	u_int32_t size ; 

	if(type == 0b00){

		
		size = (curr & 0x3F);





	}
	else if(type == 0b01){

		u_int8_t msbBits = (curr & 0x3F);

		u_int8_t lsBits = fgetc(file);

		size = (msbBits << 8) | lsBits; 


	}

	else if(type == 0b10){

		size = 0; 

		for(int i = 0 ; i < 4 ; i++){

			size = (size << 8) | fgetc(file); 

			

		}


	}

	return size; 

}


char *decodeString(FILE* file){

	int len = parseLengthEncoding(file); 

	char* val = (char*)malloc(sizeof(char) * len  + 1); 



	int start = 0 ; 

	while(start < len){

		val[start] = fgetc(file);
		start++;
	}

	val[len] = '\0';

	return val; 



}

long decodeMilliSeconds(FILE *file){

	unsigned long val = 0;


	for(int i = 0 ; i < 8 ; i++){

		unsigned long v = fgetc(file);

		val = val | (v << (8*i));



	}

 


	return val; 


}

long decodeSeconds(FILE *file){

	unsigned long val = 0; 

	for(int i = 0 ; i < 4 ; i++)

	
	{
		unsigned long v = fgetc(file);

		val = val | (v << (8 * i));

	}


	return val; 

}

/* char* parseString(char** input){

	int len = parseLen(input); 

	char* str = parseBulkString(input, len); 

	return str; 



} */

char* sendCommand(int fd , char* commands[], int commandLen){

    char* toSend = encodeArray(commands , commandLen); 

	send(fd , toSend , strlen(toSend),0);

	char* buf = (char*)malloc(MAX_SIZE); 

	char* input = buf; 

	int bytesRead = read(fd, buf , MAX_SIZE); 

	if(bytesRead == -1){
		printf("Error in reading.");

		free(buf);
		return NULL; 
	}

	if(bytesRead == 0){

		printf("No bytes were read.\n"); 
		free(buf);

		return NULL; 

	}
	
	int len = parseLen(&input);

	char* response = parseBulkString(&input, len); 


	if(response == NULL){

		printf("No response was received.\n");
		free(buf); 
		return NULL; 
	}

	printf("Response is: %s\n", response);

    



	free(buf); 

    return response; 



}