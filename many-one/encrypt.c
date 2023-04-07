
#include "encrypt.h"

long int encrypt(long int p){
        long int ret;
        asm(" mov %1, %%rax;\n"
            " xor %%fs:0x30, %%rax;"
            " rol $0x11, %%rax;"
            " mov %%rax, %0;"
        : "=r"(ret)
        : "r"(p)
        : "%rax"
        );
        return ret;
}