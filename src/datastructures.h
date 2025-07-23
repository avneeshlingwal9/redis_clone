#include "uthash.h"
#define RESP_PONG "+PONG\r\n"
#define RESP_OK "+OK\r\n"
#define RESP_NULL "$-1\r\n"

#define MAX_ARGS 1024
#define INFINITY 1000000000
#define RESP_NULL_ARRAY "*0\r\n"
typedef enum {

    PING , 
    GET , 
    SET , 
    UNKNOWN, 
    ECHO,
    

}Commands;

typedef enum{
    PX,
    OTHER, 

}Options; 

typedef struct ValueNode{

    char* value; 
    struct timespec currTime; 
    int expireTime; 

}ValueNode;
typedef struct HashMap{

    char* key;
    void* value;

    UT_hash_handle hh; 

}HashMap;



