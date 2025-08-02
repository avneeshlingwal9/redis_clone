
#include "algorithms.h"


/**
 * @brief Parses RESP Bulk string into C-string.
 *
 * @details Creates a new string and copies the string from RESP encoded to C-string. 
 *
 * @param input: The RESP encoded string.
 * 
 * @param length: The length of string to parse. 
 * 
 * @return The parsed string. 
 */


char* parseBulkString( char ** input , int length){

	char *str = (char*)malloc(length + 1);

	if(str == NULL){

		perror("Not able to allocate memory for parsing.\n"); 

		return NULL;

	}


	strncpy(str , *input , length); 

	str[length] = '\0'; 

	(*input) += length;

	(*input) += 2 ; // Skip CRLF.

	return str; 

}
/**
 * @brief Tells the length of the next string or array to come. 
 *
 * @details Parses the given string according to RESP Protocol and then calculates the length of upcoming string.
 *
 * @param input: A pointer to char*, storing the RESP array. 
 * 
 * @return Returns the length of next string.
 */

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
/**
 * @brief Parses an resp encoded array.
 *
 * @details Parses the resp encoded array and stores everything in arguments array, using parseBulkString() and parseLen(). 
 *
 * @param buf: Buffer containing the RESP encoded string.
 * 
 * @param end: The end of the buffer. 
 * 
 * @param arguments: The array in which we had to fill the arguments.
 * 
 * @param numArg: Total number of arguments.  
 * 
 * @return Returns whether the parsing was successful or not. 
 */

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
/**
 * @brief Parses string to Commands.
 *
 * @details Converts the string to enum Commands.

 * @param comm The command.

 * @return The command which is parsed.
 */
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

/**
 * @brief Encodes a C-string into RESP string. 
 *
 * @details Uses snprintf and encodes the given string according to RESP protocol.
 *
 * @param str: The string which we would like to encode.
 * 
 * @return Returns the RESP encoded string.
 */

char* encodeStr(char *str){

    int len = strlen(str);

    int size = snprintf(NULL , 0 , "$%d\r\n%s\r\n", len , str) + 1 ; 

    char* encodedStr = (char*)malloc(size);

    snprintf(encodedStr, size  , "$%d\r\n%s\r\n", (int)strlen(str) ,str);

    return encodedStr;
    

}
/**
 * @brief Converts the option C-string to Option type.
 *
 * @details Parses any string to its corresponding Option. 
 *
 * @return Returns the Options. 
 */

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
	if(strcasecmp(opt, "getack") == 0){

		return GETACK;

	}

	printf("Option %s not found.\n", opt); 

	return OTHER;

}

/**
 * @brief Encodes an C-array of string to RESP array. 
 *
 *
 * @param values The array of strings. 
 * 
 * @param numEl The number of elements in the array.
 * 
 * @return The encoded RESP array. 
 */

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

/**
 * @brief Given a binary encoded length, it converts it into int.
 *
 * @details When RDB is read, it parses the length, based on RESP protocol to int. 
 *
 * @param file: File pointer. 
 * 
 * @return The length decoded.
 */
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

/**
 * @brief Decodes the string from binary. 
 *
 * @details When RDB is read, it is used to convert keys and values from their binary to string equivalent form.
 
 * @param file The file type.
 * @return The encoded string. 
 */
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

/**
 * @brief Decodes millseconds from binary to long.
 *
 * @details Uses in reading RDB dump, for the keys with expiry time. 
 *
 * @param file: The RDB file pointer.
 * 
 * @return Milliseconds decoded as long. 
 */

long decodeMilliSeconds(FILE *file){

	unsigned long val = 0;


	for(int i = 0 ; i < 8 ; i++){

		unsigned long v = fgetc(file);

		val = val | (v << (8*i));



	}

 


	return val; 


}

/**
 * @brief Decodes seconds from binary. 
 *
 * @details Given seconds in little-endian order, converts it into long. 
 *
 * @param file: Pointing to the RDB file. 
 * 
 * @return long containing seconds.
 */
long decodeSeconds(FILE *file){

	unsigned long val = 0; 

	for(int i = 0 ; i < 4 ; i++)

	
	{
		unsigned long v = fgetc(file);

		val = val | (v << (8 * i));

	}


	return val; 

}



/**
 * @brief Function used to send command. 
 *
 * @details It is used to send a set of RESP commands, encoded as RESP array. 
 *
 * @param fd: The file descriptor in which command is to be sent.
 * 
 * @param commands : The array commands to be encoded. 
 * 
 * @param commandLen: The number of commands. 
 */
void sendCommand(int fd , char* commands[], int commandLen){

    char* toSend = encodeArray(commands , commandLen); 

	send(fd , toSend , strlen(toSend),0);
    return; 

}
/**
 * @brief Used to add a command to global leader buffer.
 *
 * @details Used by leader, as a way to store commands to propagate among its follower.
 *
 * @param arguments: The array of arguments.
 * 
 * @param numEl: The number of arguments. 
 * 

 */
void addToBuffer(char** arguments , int numEl){

    if(commandBufferOffset == MAX_COMMANDS){
        
        printf("Not able to add to buffer.\n");
        return; 
    }

    commandBuffer[commandBufferOffset++] = encodeArray(arguments, numEl);



}