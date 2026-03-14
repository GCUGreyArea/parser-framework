#ifndef NUMBERS_H
#define NUMBERS_H

#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include "Exception/Exceptions.h"

class Numbers {
public:
    /**
    * @brief Determine if the literal is a valid number
    *
    * @param s
    * @return unsigned int as the base of number, or zero if it
    * is not a valid numerical string.
    */
    static unsigned int is_number(std::string s) {
        const char *str = s.data();

        unsigned int ret = 10;

        if (*str == '0') {
            str++;
            if (*str == 'x' || *str == 'X') {
                ret = 16;
                str++;
            }
            else if (*str == 'b' || *str == 'B') {
                ret = 2;
                str++;
            }
            else {
                ret = 8;
                str++;
            }
        }

        while (*str != '\0') {
            switch (*str) {
            case '0' ... '1':
                break;
            case '2' ... '7':
                if (ret == 2)
                    return 0;
                break;
            case '8' ... '9':
                if(ret == 8) {
                    std::cerr << "WARNING: Octal number contains no octal value " << *str << std::endl;
                    std::cerr << "Falling back to decimal" << std::endl;
                    ret = 10;
                }
                break;
            case 'A' ... 'F':
            case 'a' ... 'f':
                if (ret != 16)
                    return 0;
                break;

            default:
                return 0;
            }
            str++;
        }

        return ret;
    }

    static int to_int(std::string str, unsigned int base) {
        int ret=0;
        switch(base) {
            case 2:  {
                char * end = nullptr;
                char * n = str.data()+2; // Remove "0b"
                ret = strtol(n,&end,2);
            }
            break;
            case 8:
                sscanf(str.c_str(),"%o",&ret);
                break;
            case 10:
                sscanf(str.c_str(),"%d",&ret);
                break;
            case 16:
                sscanf(str.c_str(),"%x",&ret);
                break;

            default:
                throw Exception::NotAValidBase(std::to_string(base));
        }

        return ret;
    }
};

#endif//NUMBERS_H