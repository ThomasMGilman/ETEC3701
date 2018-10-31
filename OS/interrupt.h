#pragma once

unsigned char kernelStack[500];
unsigned ring0StackInfo[] =
{
    0,
    (unsigned)(kernelStack+sizeof(kernelStack)),
    1<<3
};

#pragma pack(push,1)
struct IDTEntry{
    unsigned short addrLow;
    unsigned short selector;//code segment
    unsigned char zero;     //must be zero
    unsigned char flags;    //0x8e for interrupt handler
    unsigned short addrHigh;
};
#pragma pack(pop)

#pragma pack(push,1)
struct GDTEntry{
    unsigned short limitLow;
    unsigned char base0, base1, base2;
    unsigned short flagsAndLimitHigh;
    unsigned char base3;
};
#pragma pack(pop)

#pragma pack(push,1)
struct LGDT{
    unsigned short size;
    struct GDTEntry* addr;
};
#pragma pack(pop)

#pragma pack(push,1)
struct InterruptFrame{
    unsigned eip;
    unsigned cs;
    unsigned eflags;
    unsigned esp;   //only used when undergoing
    unsigned ss;    //a ring transition
};
#pragma pack(pop)

#pragma pack(push,1)
struct LIDT{
    unsigned short size;
    struct IDTEntry* addr;
};
#pragma pack(pop)

int syscall(int p0, int p1, int p2, int p3);

int exec(const char* filename);

void haltForever(void);

void haltUntilInterrupt(void);

void setupPICS_RTC(unsigned rate);

void table(int i, void* func);

void interrupt_init(void);