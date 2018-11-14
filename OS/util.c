#include "util.h"

int ksprintf(char* s, const char* fmt, ... ) __attribute__((format (printf , 2, 3 ) ));
char debugMsg[100];

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
    int divisor;
    unsigned v;
    logString("playing\n");
    divisor = (freq > 0) ? (1193180 / freq) : 0;
    ksprintf(debugMsg,"div:%d, numPass:%d\n", divisor, freq);
    logString(debugMsg);
    outb(0x43, 0xb6);
    outb(0x42, (const unsigned)(divisor & 0xff));   //low byte
    outb(0x42, (const unsigned)(divisor >> 8)); //high byte
    v = inb(0x61);
    ksprintf(debugMsg,"CurDiv:%d, low:%d, high:%d, Freq:%d, v:%d\n",divisor, divisor&0xff, divisor&0xff00, freq, v);
    logString(debugMsg);
    if((v & 3) != 3)
        outb(0x61, (v|3));
    logString("\n");
}

void sleep(int waitTime)
{
    unsigned v, timeToWait;
    timeToWait = waitTime * (Frequency) * 100;          //wait time
    while(timeToWait--){;}
    logString("done waiting\n");
    v = inb(0x61);
    outb(0x61, (v & 0xfffc));                           //turn off the lower two bits
}

int Factorial(int num)
{
    unsigned retValue = 1, index;
    for(index = 1; index <= num; index++)
        retValue *= index;
    return retValue;
}