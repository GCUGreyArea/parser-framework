#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "string.h"

/**
 * @brief Create a new string by allocating memory 
 * through malloc and copying the content of the 
 * input string. 
 * 
 * @param str 
 * @return char* 
 */
char * copy_string(char * str) {
    if(str == NULL) {
      	return NULL;
    }

    int len = strlen((const char*) str);

	char * ret = malloc(len+1);
	strcpy(ret,str);
	ret[len] = '\0';

	return ret;
}

bool cmp_string(char * str1,char * str2) {
    int l1 = strlen((const char*)str1);
    int l2 = strlen((const char*) str2);

    if(l1 == l2) {
        for(int i = 0; i < l1; i++) {
            if(str1[i] != str2[i]) {
                return false;
            }
        }
        
        return true;
    }

    return false;
}

void kill_string(char * str) {
    memset(str,0,strlen(str));
    free(str);
}