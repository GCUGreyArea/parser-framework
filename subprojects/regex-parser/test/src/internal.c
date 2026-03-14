/**
 * @file internal.c
 * @author barry Robinson (barry.w.robinson64@dgmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-08-31
 * 
 * @copyright Copyright (c) 2021
 * @addtogroup library 
 * @{
 * @addtogroup internal
 * @{
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "internal.h"


void * _alloc_mem_(size_t size, const char * func, const char * file, int line) {
    void * mem = malloc(size);
    if(mem == NULL) {
        fprintf(stderr,"Failed to allocte memory [bytes %ld / file %s / function %s / line %d]\n", size, file,func,line);
        return NULL;
    }

    return mem;
}

/**
 * @}
 * @} 
 */