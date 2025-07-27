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

/* void createEmptyRDB(){
    char* contents = "524544495330303131fa0972656469732d76657205372e322e30fa0a72656469732d62697473c040fa056374696d65c26d08bc65fa08757365642d6d656dc2b0c41000fa08616f662d62617365c000fff06e3bfec0ff5aa2";
    FILE* file = fopen("empty.rdb", "wb");

    if(file == NULL){

        perror("Not able to open file.\n");
        return; 
    }
    
    fprintf(file, contents);

    fclose(file);


}

char hexToBinary(char input){

    switch(input){

        case '0': 
            return '0';


    }
    




} */

