/*
Code written by Thomas Gilman for Operating Systems One taught James Hudson.
Some functions and implementations of code use code from the OS slides,
the rest was written by Thomas Gilman.
*/
#pragma once

#define kStackSize 4096

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
struct LIDT{
    unsigned short size;
    struct IDTEntry* addr;
};
#pragma pack(pop)

#pragma pack(push,1)
struct ScanCode{
    char lowerVal;
    char upperVal;
    unsigned int printable;
    unsigned int keyPressed;
};
#pragma pack(pop)

int keyboard_getline(char* buffer, unsigned num);

void show_cursor(void);

void keyboard_interrupt();

void keyHandler(unsigned keyValIn);

// static void send(unsigned short port, unsigned char val);

// static unsigned char recv();

void table(int i, void* func);

void setupPICS_RTC(unsigned rate);

void setupKeyBoard(void);

void setupGDT(void);

void syscall_handler(unsigned* ptr);

int exec(const char* filename);

int interrupt_init(void);