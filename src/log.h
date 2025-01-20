#include <stdio.h>
#ifndef LOG
    #define LOG(cexprarg, ...) printf("INFO: "cexprarg"\n", ##__VA_ARGS__)
#endif

#ifndef ERR
    #define ERR(cexprarg, ...) printf("ERROR: "cexprarg"\n", ##__VA_ARGS__)
#endif