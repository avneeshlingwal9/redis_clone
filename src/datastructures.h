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

#define EMPTYRDB "524544495330303131fa0972656469732d76657205372e322e30fa0a72656469732d62697473c040fa056374696d65c26d08bc65fa08757365642d6d656dc2b0c41000fa08616f662d62617365c000fff06e3bfec0ff5aa2"


#define MAX_COMMANDS 1024
#define MAX_SIZE 2056
#define MAX_REPLICA 32
#define MAX_PARENT_BUFFER 1048576
char* dir  = NULL; 

char* dbfilename = NULL; 

char* filePath = NULL; 

bool isMaster; 

char* parent = NULL; 

char* replicationId = NULL; 

int replicationOffset; 

char* commandBuffer[MAX_COMMANDS]; 

int commandBufferOffset = 0; 

int replicaList[MAX_REPLICA];

int replicaOffset = 0; 

char parentBuf[MAX_PARENT_BUFFER];
int parentOffset = 0 ;
int parentCommand = 0;


typedef enum {

    PING , 
    GET , 
    SET , 
    UNKNOWN, 
    ECHO,
    CONFIG,
    KEYS,
    INFO,
    REPLCONF,
    PSYNC,
    

}Commands;

typedef enum{
    PX,
    REPLICATION, 
    OTHER, 
    LISTENINGPORT,
    CAPA, 
    PSYNC2,
    GETACK,



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



