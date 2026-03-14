#ifndef _INTERNAL_
#define _INTERNAL_

#include <stdlib.h>


/**
 * @brief Internal functions and macros to report on memory failure
 *
 * @param amount The of memory to allocate as defiend by size_t
 * @param func   The calling function
 * @param file   The source file
 * @param line   The line the macro was invoked at
 * @return void* The memory requested, or NULL if unsucessful
 * 
 * @addtogroup libnrary
 * @{
 * @addtogroup internal
 * @{
 */

#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

void * _alloc_mem_(size_t amount,const char * func, const char * file, int line);
#define alloc_mem(X) _alloc_mem_(X,__FUNCTION__,__FILE__,__LINE__)

/**
 * @brief Print error message with acompanying file and line detail
 *
 * @param fmt format for message
 * @param ... Variable argument list
 */
extern void print_err(const char *fmt, ...);
#define print_err(fmt, ...) fprintf(stderr,"ERROR [file %s / func %s / line %d]: " fmt, __FILE__,__FUNCTION__,__LINE__, ##__VA_ARGS__)

#ifdef __cplusplus
}
#endif//__cplusplus

/**
 * @}
 * @}
 * 
 */

#endif//_INTERNAL_