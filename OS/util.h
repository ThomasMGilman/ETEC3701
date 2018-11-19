#pragma once

#define QEMUPORT 0x3f8

extern volatile unsigned jiffies;
extern unsigned Frequency;

#pragma pack(push,1)
    struct MultibootInfo
    {
        unsigned flags;
        unsigned memLower;  //always 640K
        unsigned memUpper;  //amount of memory above 1MB
        unsigned long long mbiFramebufferAddress;
        unsigned mbiFramebufferPitch;   //The distance from the start of first address of pixel and next pixel address
        unsigned mbiFramebufferWidth;
        unsigned mbiFramebufferHeight;
        unsigned char mbiFramebufferBpp;    //bits per pixel, (often 16 or 24bits (truecolor))
        unsigned char mbiFramebufferType;
        unsigned char mbiFramebufferRedPos;
        unsigned char mbiFramebufferRedMask;
        unsigned char mbiFramebufferGreenPos;
        unsigned char mbiFramebufferGreenMask;
        unsigned char mbiFramebufferBluePos;
        unsigned char mbiFramebufferBlueMask;
    };
#pragma pack(pop)

void haltForever(void);

void haltUntilInterrupt(void);

void sti(void);

void outb(unsigned short port, unsigned char value);

void outw(unsigned short port, unsigned short value);

unsigned char inb(unsigned short port);

unsigned short inw(unsigned short port);

void logString(char* myString);

void clearBss(char* bssStart, char* bssEnd);

int kstrln(char *string);

int kmemcmp(void *p, void *q, int n);

void kmemcpy(void *dst, const void *src, unsigned num);

void kmemset(void *dstV, unsigned num);

void playSound(int freq);

void sleep(int waitTime);

void silence(void);

int Factorial(int num);