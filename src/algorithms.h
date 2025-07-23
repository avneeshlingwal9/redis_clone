#include "datastructures.h"
#include <time.h>
#include <stdio.h>

HashMap *dictHead = NULL;

double getTimeDifference(struct timespec begin){

	struct timespec curr; 

	timespec_get(&curr , TIME_UTC); 

	double diff = (curr.tv_sec - begin.tv_sec)* 1000.0 + (curr.tv_nsec - begin.tv_nsec)/1000000.0;

	return diff; 


}



void setValue(char* key , char* aValue , int expiry){

    ValueNode* value = (ValueNode*)malloc(sizeof(ValueNode)); 

    if(value == NULL){

        printf("Not able to allocate memory.\n"); 
        return ;
    }

    value->value = strdup(aValue);
    
    timespec_get(&(value->currTime) , TIME_UTC);

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