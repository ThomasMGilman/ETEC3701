#pragma once

typedef struct va_list_{
    char* x;
}va_list;

#define va_start(VarList, List) \
    VarList.x = ((char*)&List)+sizeof(List)

#define va_arg(VarList, type) \
    (*((type*)_va_arg(&VarList, sizeof(type))))

#define va_end(VarList)

void* _va_arg(va_list* VarList, unsigned size);