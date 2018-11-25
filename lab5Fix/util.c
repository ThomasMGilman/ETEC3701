#include "util.h"

void clearBss(char* bssStart, char* bssEnd)
{
    while(bssStart++ != bssEnd) 
        *bssStart = 0;
}

int Factorial(int num)
{
    int retValue = 1;
    for(int index = 1; index <= num; index++)
        retValue *= index;
    return retValue;
}