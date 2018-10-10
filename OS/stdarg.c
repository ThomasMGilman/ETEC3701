#include "stdarg.h"

void* _va_arg(va_list* VarList, unsigned size)
{
    void* tmp = VarList->x;
    VarList->x += size;
    return tmp;
}