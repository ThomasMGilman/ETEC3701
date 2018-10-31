#include "syscalls.h"
#include "errno.h"
#include "interrupt.h"
#include "kprintf.h"

#define INTERRUPT_SIZE 49

const char* FileToLoad = "linkerscript2.txt";
char debugMsg[100];
volatile unsigned jiffies = 0;

void logString(char* myString);                     //UTIL 
void outb(unsigned short port, unsigned char val);  //DISK WRITE
unsigned char inb(unsigned short port);             //DISK READ

//FILE STUFF
int file_open(const char* fileName, int flags);
int file_close(int fd);
int file_read(int fd, void* buf, int count);

struct GDTEntry gdt[] = {
    { 0,0,0,0,0,0 },            //zeros
    { 0xffff, 0,0,0, 0xcf92, 0},//data, ring 0
    { 0xffff, 0,0,0, 0xcf9a, 0},//code, ring 0
    { 0xffff, 0,0,0, 0xcff2, 0},//data, ring 3
    { 0xffff, 0,0,0, 0xcffa, 0},//code, ring 3
    { 0,0,0,0,0,0 }             //task selector
};

struct IDTEntry idt[INTERRUPT_SIZE];

int exec(const char* filename)
{   
    unsigned fd;
    if((fd = file_open(filename,0)) < 0)
    {
        ksprintf(debugMsg,"FileName:%s failed to open!! failed to execute!!\n",filename);
        logString(debugMsg);
        return fd;
    }
    //put into memory at &0x400000
    if((fd = file_close(fd)) < 0)
    {
        ksprintf(debugMsg,"FileName:%s failed to close!! failed to execute!!\n",filename);
        logString(debugMsg);
        return fd;
    }
    return 0;
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

/*
    RATE_VALUES:
    15=2Hz, 14=4Hz, 13=8Hz, 12=16Hz, 11=32Hz, 10=64Hz, 9=128Hz, 
    8=256Hz, 7=512Hz, 6=1024Hz, 5=2048Hz, 4=4096Hz, 3=8192Hz
*/
void setupPICS_RTC(unsigned rate)
{
    unsigned tmp;
    //first PIC
    outb(0x20, 0x11);    //want to setup
    outb(0x21, 32);      //base for interrupts
    outb(0x21, 4);       //your prime PIC
    outb(0x21, 1);       //use 8086 conventions
    outb(0x21, 0);       //Enable all 8 IRQ lines
    //second PIC
    outb(0xa0, 0x11);    //want to setup
    outb(0xa1, 40);      //base for interrupts
    outb(0xa1, 2);       //secondary PIC
    outb(0xa1, 1);       //use 8086
    outb(0xa1, 0);       //all 8 IRQ lines
    //setup RTC
    outb(0x70, 0xa);    //RTC we want acces rate register
    tmp = inb(0x71);    //old val
    outb(0x70, 0xa);    //access rate reg again
    if(rate < 3)        rate = 3;
    else if(rate > 15)  rate = 15;
    outb(0x71, rate|(tmp&0xf0)); //write rate
    outb(0x70, 11);     //access reg 11 RTC chip & reset val
    tmp = inb(0x71);    //prev val
    outb(0x70, 11);     //select reg 11
    outb(0x71, tmp|0x40);       //Enable interrupts
}

void syscall_handler(unsigned* ptr)
{
    switch(ptr[0])
    {
        case SYSCALL_READ:
            if( ptr[2] < 0x400000 || ptr[2] > 0x800000 ||
                ptr[2] + ptr[3] < 0x400000 || ptr[2] + ptr[3] > 0x800000)
            {
                ptr [0] = -EFAULT;
                break;
            }
            ///do stuff
            break;
        case SYSCALL_WRITE:
            if( ptr[2] < 0x400000 || ptr[2] > 0x800000)
            {
                ptr [0] = -EFAULT;
                break;
            }
            if(ptr[3] < 0 || ptr[2]+ptr[3] >= 0x800000)
            {
                ptr[0] = -EINVAL;
                break;
            }
            ptr[0] = file_write(ptr[1],(char*)ptr[2],ptr[3]);
            break;
        case SYSCALL_OPEN:
            ptr[0] = file_open((char*)ptr[1], ptr[2]);
            break;
        case SYSCALL_CLOSE:
            ptr[0] = file_close(ptr[1]);
            break;
        case SYSCALL_EXIT:
            break;
        default:
            ptr[0] = -ENOSYS;
            break;
    }
}

__attribute__((interrupt))
void divideByZeroInterrupt(struct InterruptFrame* fr)
{
    kprintf("\nERROR: Division by Zero is undefined!\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void debugTrapInterrupt(struct InterruptFrame* fr)
{
    kprintf("\nERROR: Debug Trap!\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void badOpcodeInterrupt(struct InterruptFrame* fr)
{
    kprintf("\nERROR: Bad opcode!\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void unknownInterrupt(struct InterruptFrame* fr)
{
    kprintf("\nERROR: Fatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void unknownInterruptWithCode(struct InterruptFrame* fr, unsigned code)
{
    kprintf("\nERROR: Fatal exception: Code=%x eip=%x\n", code, fr->eip);
    haltForever();
}

__attribute__((interrupt))
void protectionFaultInterrupt(struct InterruptFrame* fr, unsigned code)
{
    kprintf("\nERROR: Protection Fault!\nFatal exception: Code=%x eip=%x\n", code, fr->eip);
    haltForever();
}

__attribute__((interrupt))
void pageFaultInterrupt(struct InterruptFrame* fr, unsigned code)
{
    kprintf("\nERROR: Page Fault!\nFatal exception: Code=%x eip=%x\n", code, fr->eip);
    haltForever();
}

__attribute__((__interrupt__))
void int48Interrupt(struct InterruptFrame* fr)
{
    if( fr->esp < 0x400000 || fr->esp > 0x800000-(4*4)) //Invalid parameter. Ignore the system call.
        return;
    unsigned* espCheck = (unsigned*)fr->esp;
    syscall_handler(espCheck);
}

//Framework for interrupt handlers: 0...7
__attribute__((__interrupt__))
void firstPicInterrupt(struct InterruptFrame* fr){
outb( 0x20, 32 );
}
//Framework for interrupt handlers: 8...15
__attribute__((__interrupt__))
void secondPicInterrupt(struct InterruptFrame* fr){
outb( 0x20, 32 );
outb( 0xa0, 32 );
}
//table(40,RTCint);
__attribute__((__interrupt__))
void RTCint(struct InterruptFrame* fr){
    outb(0x70, 0xc);
    inb(0x71);
    outb(0x20, 30);
    outb(0xa0, 32);
    jiffies++;
}

void setInterruptTable(void)
{
    unsigned index;
    for(index = 0; index < INTERRUPT_SIZE; index++)
    {
        if(index == 0)
            table(index, divideByZeroInterrupt);
        else if( index == 3)
            table(index, debugTrapInterrupt);
        else if(index == 6)
            table(index, badOpcodeInterrupt);
        else if(index == 8 || index == 17 ||
        (index >= 10 && index < 13))
            table(index, unknownInterruptWithCode);
        else if(index == 13)
            table(index, protectionFaultInterrupt);
        else if(index == 14)
            table(index, pageFaultInterrupt);
        else if(index == 48)
        {
            table(index, int48Interrupt);
            idt[48].flags = 0xee;
        }
        else
            table(index, unknownInterrupt);
    }
}

void table(int i, void* func){
    unsigned x = (unsigned)func;
    idt[i].addrLow = x&0xffff;
    idt[i].selector = 2 << 3;
    idt[i].zero = 0;
    idt[i].flags = 0x8e;
    idt[i].addrHigh = x>>16;
}

void interrupt_init(void)
{
    struct LGDT lgdt;
    unsigned index;
    unsigned tmp = (unsigned)ring0StackInfo;
    exec(FileToLoad);
    setupPICS_RTC(6);
    gdt[5].limitLow = sizeof(ring0StackInfo);
    gdt[5].base0 = tmp & 0xff;
    gdt[5].base1 = (tmp>>8) & 0xff;
    gdt[5].base2 = (tmp>>16) & 0xff;
    gdt[5].flagsAndLimitHigh = 0x00e9;
    gdt[5].base3 = (tmp>>24) & 0xff;
    lgdt.size = sizeof(gdt);
    lgdt.addr = &gdt[0];
    asm volatile( "lgdt [eax]\n"
        : //no outputs
        :   "a"(&lgdt),     //put address of gdt in eax
            "b"((5<<3)|3)   //put task register index in ebx
        : "memory" );
    setInterruptTable();
    struct LIDT tmp;
    tmp.size = sizeof(idt);
    tmp.addr = &idt[0];
    asm volatile("lidt [eax]" : : "a"(&tmp) : "memory" );
    asm volatile("sti" : : : "memory");
}