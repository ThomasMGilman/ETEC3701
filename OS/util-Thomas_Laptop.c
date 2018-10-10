#include "util.h"

void clearBss(char* bssStart, char* bssEnd)
{
    while(bssStart++ != bssEnd) 
        *bssStart = 0;
}

void outb(unsigned short port, unsigned char value)
{
    asm volatile("out dx, al" : : "a"(value), "d"(port) : "memory"); //al is 15bit register
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

int isBusy()
{
    return inb(0x1f7) & 0x80; //come back as 0: not busy, or 1: busy
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

//if scrolling kmemcpy(frambuffer, frambuffer+CHAR_HEIGHT*Pitch, (Screen_Height - CHAR_HEIGHT)*Pitch);
//clear last line kmemset();