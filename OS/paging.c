#include "paging.h"
#define DEVICE_MEMORY ( (1<<3) | (1<<4) )
#define PRESENT 1
#define MUST_BE_ONE (1<<7)
#define USER_ACCESS (1<<2)
#define WRITEABLE (1<<1)

page_table[1024] __attribute__((aligned(4096)));

void set_page_table(void* p){
    asm volatile( "mov cr3,eax"
    :
    : "a"( (unsigned)p )
    : "memory" );
}

void enable_paging(){
    asm volatile(
    "mov eax,cr4\n"
    "or eax,16\n"
    "mov cr4,eax\n"
    "mov eax,cr0\n"
    "or eax, 0x80010000\n"
    "mov cr0,eax\n"
    "jmp 1f\n"
    "1: nop" //gas syntax: local label
    ::: "eax","memory");
}


void memory_init()
{
    ;
}