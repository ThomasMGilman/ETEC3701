/*
Code written by Thomas Gilman for Operating Systems One taught James Hudson.
Some functions and implementations of code use code from the OS slides,
the rest was written by Thomas Gilman.
*/
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

void sti(void)
{
    asm volatile("sti":::"memory");
}

void outb(unsigned short port, unsigned char value)
{
    asm volatile("out dx, al" : : "a"(value), "d"(port) : "memory"); //al is 8bit register
}
void outw(unsigned short port, unsigned short value)
{
    asm volatile("out dx, ax" : : "a"(value), "d"(port) : "memory"); //ax is 16bit register
}

char inb(unsigned short port)
{
    int value;
    asm volatile("in al, dx" : "=a"(value): "d"(port) );
    return (char) value;
}

unsigned short inw(unsigned short port)
{
    unsigned value;
    asm volatile("in ax, dx" : "=a"(value): "d"(port) );
    return (unsigned short) value;
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

void playSound(int freq)
{
    unsigned v;
    int divisor = (freq > 0) ? (1193180 / freq) : 0;
    outb(0x43, 0xb6);
    outb(0x42, (const unsigned)(divisor & 0xff));   //low byte
    outb(0x42, (const unsigned)(divisor >> 8));     //high byte
    v = inb(0x61);
    if(freq == 0)
        outb(0x61, v & ~3);
    else if((v & 3) != 3)
    {
        outb(0x61, (v|3));
    }
}

void sleep(int waitTime)
{
    while(jiffies < waitTime)
    {
        sti();
        haltUntilInterrupt();
    }
    jiffies = 0;
    logString("done waiting\n");
}

void silence(void)
{
    unsigned v = inb(0x61) & 0xFC;
    outb(0x61, v);
    logString("Silenced\n");
}

int Factorial(int num)
{
    unsigned retValue = 1, index;
    for(index = 1; index <= num; index++)
        retValue *= index;
    return retValue;
}