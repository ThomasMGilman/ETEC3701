#pragma once

#define kStackSize 500

unsigned char kernelStack[kStackSize];

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

void table(int i, void* func);

void setupPICS_RTC(unsigned rate);

void setupGDT(void);

void interrupt_init(void);