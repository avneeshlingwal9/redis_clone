#include "uthash.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#define RESP_PONG "+PONG\r\n"
#define RESP_OK "+OK\r\n"
#define RESP_NULL "$-1\r\n"

#define MAX_ARGS 1024
#define INFINITY 1000000000
#define RESP_NULL_ARRAY "*0\r\n"

#define MAX_CONNECTIONS 32


#define MAX_SIZE 2056

char* dir  = NULL; 

char* dbfilename = NULL; 

char* filePath = NULL; 

bool isMaster; 

char* parent = NULL; 

char* replicationId = NULL; 

int replicationOffset; 

typedef enum {

    PING , 
    GET , 
    SET , 
    UNKNOWN, 
    ECHO,
    CONFIG,
    KEYS,
    INFO,
    

}Commands;

typedef enum{
    PX,
    REPLICATION, 
    OTHER, 

}Options; 

typedef struct ValueNode{

    char* value; 
    struct timespec currTime; 
    double expireTime; 

}ValueNode;
typedef struct HashMap{

    char* key;
    void* value;

    UT_hash_handle hh; 

}HashMap;



