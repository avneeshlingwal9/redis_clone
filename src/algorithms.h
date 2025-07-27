#include "datastructures.h"
#include <time.h>


HashMap *dictHead = NULL;

bool fileExists(char* path){

    FILE* file = fopen(path , "rb");

    if(file ){

        fclose(file);
        return true;

    }

     

    return false; 


}


double getTimeDifference(struct timespec begin){

	struct timespec curr; 

	timespec_get(&curr , TIME_UTC); 

	double diff = (curr.tv_sec - begin.tv_sec)* 1000.0 + (curr.tv_nsec - begin.tv_nsec)/1000000.0;

	return diff; 


}



void setValue(char* key , char* aValue , double expiry , bool unixTime){

    ValueNode* value = (ValueNode*)malloc(sizeof(ValueNode)); 

    if(value == NULL){

        printf("Not able to allocate memory.\n"); 
        return ;
    }

    value->value = strdup(aValue);
    
    timespec_get(&(value->currTime) , TIME_UTC);

    if(unixTime){

        u_int64_t now = time(NULL);
    
        now = now* 1000; // Convert into millseconds. 

        expiry = expiry - now; 

    }


    value->expireTime = expiry;

    HashMap* curr = (HashMap*)malloc(sizeof(HashMap));

    if(curr == NULL){

        printf("Not able to allocate memory.\n"); 

        return; 

    }




    curr->key = strdup(key);
    
    curr->value = (void*)value;




    HASH_ADD_STR(dictHead , key , curr); 





}

char* getValue(char* key){

    HashMap* node ; 

    HASH_FIND_STR(dictHead , key , node);

    if(node == NULL){
        return NULL; 
    }


    ValueNode* value = (ValueNode*)node->value;

    if(value->expireTime == -1){

        return strdup(value->value);

    }

    else if(value->expireTime <= 0){

        return NULL;
    }

    else{

        if(getTimeDifference(value->currTime) < value->expireTime){

            return strdup(value->value);
        }

    }

    return NULL; 



}

void freeValue(void* node){

    ValueNode* valueNode = (ValueNode*)node; 

    free(valueNode->value);

    free(valueNode);

}

void freeDictionary(){


    HashMap* curr, *temp;
    HASH_ITER(hh , dictHead , curr , temp){

        HASH_DEL(dictHead , curr);

        free(curr->key); 

        freeValue(curr->value); 

        free(curr); 

    } 



}



int getKeys(char*** keys){


    int countKeys = HASH_COUNT(dictHead);

    if(countKeys == 0){

        *keys = NULL; 

        return 0;
    }

    char** keySet = (char**)malloc(sizeof(char*)* countKeys); 


    int start = 0 ;

    HashMap *curr; 

    for(curr = dictHead ; curr != NULL ; curr = curr->hh.next){

        keySet[start++] = strdup(curr->key);

    }

    
    *keys = keySet; 

    return countKeys;






}

int sendRDB(int fd){
    int numBytes = strlen(EMPTYRDB);
		int numChar = numBytes/2;

		char fileContents[numChar]; 

		for(int i = 0 ; i < numBytes - 1; i+= 2){

			char curr[] = {EMPTYRDB[i], EMPTYRDB[i + 1], '\0'};
			// Converting hex. 

			char byte = (char)strtol(curr , NULL, 16);
			fileContents[i/2] = byte; 
		}

		numChar = strlen(fileContents); 

		int digitLen = snprintf(NULL, 0 , "%d", numChar); 
		char* toSend = (char*)malloc(digitLen + 4 + numChar);

        if(toSend == NULL){
            printf("Not able to allocate memory for sending RDB data.\n"); 
            return 1; 
        }
		
		sprintf(toSend, "$%d\r\n%s", numChar, fileContents);

		send(fd , toSend, strlen(toSend), 0); 

		free(toSend);
}







