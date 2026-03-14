#ifndef HASH_H
#define HASH_H

#if __cplusplus // C++ programs need to know that no name mangling should occur
extern "C" {
#endif

unsigned int hash(const char* str, unsigned len);
unsigned int merge_hash(unsigned left, unsigned right);

#if __cplusplus
}               // end the extern "C" block
#endif

#endif//HASH_H