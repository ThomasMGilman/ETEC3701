#include "util.h"

void haltForever(void)
{
    while(1){
        asm volatile("hlt" ::: "memory");
    }
}

void haltUntilInterrupt(void)
{
    asm volatile("hlt" ::: "memory" );
}

void outb(unsigned short port, unsigned char value)
{
    asm volatile("out dx, al" : : "a"(value), "d"(port) : "memory"); //al is 8bit register
}
void outw(unsigned short port, unsigned short value)
{
    asm volatile("out dx, ax" : : "a"(value), "d"(port) : "memory"); //ax is 16bit register
}

unsigned char inb(unsigned short port)
{
    unsigned value;
    asm volatile("in al, dx" : "=a"(value): "d"(port) );
    return (unsigned char) value;
}

unsigned short inw(unsigned short port)
{
    unsigned value;
    asm volatile("in ax, dx" : "=a"(value): "d"(port) );
    return (unsigned short) value;
}

int syscall(int p0, int p1, int p2, int p3)
{
    asm volatile(
        "push edx\n"
        "push ecx\n"
        "push ebx\n"
        "push eax\n"
        "int 48\n"
        "pop eax\n"
        "add esp,12"
        : "+a"(p0)
        : "b"(p1), "c"(p2), "d"(p3)
    );
    return p0;
}

void logString(char* myString)
{
	int indexOfString = 0;
	while(myString[indexOfString])
		outb(QEMUPORT,myString[indexOfString++]);
}

void clearBss(char* bssStart, char* bssEnd)
{
    while(bssStart++ != bssEnd) 
        *bssStart = 0;
}

int kstrln(char *string)
{
    int len = 0;
    while(*string++)
        len++;
    return len;
}

int kmemcmp(void *p, void *q, int n)
{
    char *firstObj  = (char*)p;
    char *secObj    = (char*)q;
    while(n>0)
    {
        if(*firstObj < *secObj) return -1;
        if(*firstObj > *secObj) return 1;
        firstObj++; secObj++;
        n--;
    }
    return 0;
}

void kmemcpy(void *dstV, const void *srcV, unsigned num)
{
    char* dst = (char*)dstV;    char* src = (char*)srcV;
    while(num--)
        *dst++ = *src++;

}

void kmemset(void *dstV, unsigned num)
{
    char* dst = (char*)dstV;
    while(num--)
        *dst++ = 0x0;
}

int Factorial(int num)
{
    unsigned retValue = 1, index;
    for(index = 1; index <= num; index++)
        retValue *= index;
    return retValue;
}