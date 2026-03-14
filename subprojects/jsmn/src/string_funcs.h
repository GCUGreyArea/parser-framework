#ifndef STRING_HEADER
#define STRING_HEADER

// We are mixing C and C++ because of lex and bison 
#if __cplusplus // C++ programs need to know that no name mangling should occur
extern "C" {
#endif

#include <stdbool.h>

char * copy_string(char * str);
bool cmp_string(char * str1,char * str2);
void kill_string(char * str);

#if __cplusplus
}               // end the extern "C" block
#endif


#endif // ! STRING_HEADER