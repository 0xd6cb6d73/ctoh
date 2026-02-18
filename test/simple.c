#include <stdlib.h>

struct my_str{
    int len;
    char* buf;
};

typedef my_str my_str2;

enum e {
    a,
    b = 1,
};

// https://github.com/trustedsec/CS-Situational-Awareness-BOF/blob/master/src/common/base.c
int bofstart(char* output, int bufsize, int* currentoutsize)
{   
    output = (char*)calloc(bufsize, 1);
    *currentoutsize = 0;
    return 1;
}