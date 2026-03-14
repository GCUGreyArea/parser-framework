/**
 * @file testUtils.cpp
 * @author Barry Robinson (barry.w.robinson64@gmail.com)
 * @brief Test the utility functions
 * @version 0.1
 * @date 2021-08-31
 * 
 * @copyright Copyright (c) 2021
 * @addtogroup test
 * @{
 * @addtogroup utillities
 * @{
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <assert.h>
#include <string.h>


#include <string>

// #include <framework/libframework.h>
#include "internal.h"

void randomiseString(char * str) {
    srand(time(NULL));

    size_t size = strlen(str);

    for(int i=0;i<(int)size;i++) {
        int s = rand() % size;
        int e = rand() % size;

        assert((size_t)s < size);
        assert((size_t)e < size);

        char sc = str[s];
        char se = str[e];

        str[s] = se;
        str[e] = sc;
    }
}

void buildRandomTestFile(const char * fillPathAndName, const char * testFrase, int size) {
    if(fillPathAndName == NULL) {
        fprintf(stderr,"ERROR: File name cannot be null\n");
        return;
    }

    FILE * fp = fopen (fillPathAndName, "w");
    if(fp == NULL) {
        print_err("ERROR: Failed to open [file %s]\n", fillPathAndName);
        return;
    }

    size_t strSize = strlen(testFrase);
    char * buffer =  (char*) alloc_mem(size+1);
    if(buffer == NULL) {
        print_err("ERROR: Failed to allocate buffer for randome write of message\n");
        return;
    }

    strcpy(buffer,testFrase);

    for(int i=0;i<(int)strSize;i++) {
        fputs(buffer,fp);
        fputs("\n",fp);

        randomiseString(buffer);
       fputs(buffer,fp);
    }

    free(buffer);
}



void buildNormalTestFile(const char * fillPathAndName, const char * testFrase, size_t buffSize, int writeTimes, bool addIdx) {
    if(fillPathAndName == NULL) {
        fprintf(stderr,"ERROR: File name cannot be null\n");
        return;
    }

    FILE * fp = fopen (fillPathAndName, "w+");
    if(fp == NULL) {
        print_err("ERROR: Failed to open [file %s]\n", fillPathAndName);
        return;
    }

    for(int i=0;i<writeTimes;i++){
        for(size_t j=0;j<buffSize;j++) {
            fputc(testFrase[j],fp);
        }

        if(addIdx) {
            fputs(std::to_string(i).c_str(),fp);
        }
    }

    fclose(fp);
}

/**
 * @}
 * @}
 */
