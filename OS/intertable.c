/*
Code written by Thomas Gilman for Operating Systems One taught James Hudson.
Some functions and implementations of code use code from the OS slides,
the rest was written by Thomas Gilman.
*/
#include "intertable.h"
#include "syscalls.h"

void haltForever(void);
void haltUntilInterrupt(void);
void outb(unsigned short port, unsigned char value);
char inb(unsigned short port);
int kprintf(const char* fmt, ... ) __attribute__((format (printf , 1, 2 ) ));
int ksprintf(char* s, const char* fmt, ... ) __attribute__((format (printf , 2, 3 ) )); 
void keyHandler(unsigned keyValIn);
void syscall_handler(int* ptr);
void logString(char* myString);

char debugMsg[50];

__attribute__((interrupt))
void divideByZeroInterrupt(struct InterruptFrame* fr)                   //interrupt 0
{
    kprintf("\nERROR: Division by Zero is undefined!\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void debugTrapInterrupt(struct InterruptFrame* fr)                      //interrupt 1
{
    kprintf("\nERROR: Debug Trap!\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void NMIInterrupt(struct InterruptFrame* fr)                            //interrupt 2
{
    kprintf("\nERROR: NMI!!\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void int3Interrupt(struct InterruptFrame* fr)                           //interrupt 3
{
    kprintf("\nERROR: int3 Trap!\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void OverflowInterrupt(struct InterruptFrame* fr)                       //interrupt 4
{
    kprintf("\nERROR: Overflow!!\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void BCInterrupt(struct InterruptFrame* fr)                             //interrupt 5
{
    kprintf("\nERROR: BoundCheck!!\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void badOpcodeInterrupt(struct InterruptFrame* fr)                      //interrupt 6
{
    kprintf("\nERROR: Bad opcode!\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void NFPUInterrupt(struct InterruptFrame* fr)                           //interrupt 7
{
    kprintf("\nERROR: NO FPU!\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void DFInterrupt(struct InterruptFrame* fr, unsigned code)              //interrupt 8
{
    kprintf("\nERROR: DoubleFault exception: Code=%x eip=%x\n", code, fr->eip);
    haltForever();
}

__attribute__((interrupt))
void FPUOInterrupt(struct InterruptFrame* fr)                           //interrupt 9
{
    kprintf("\nERROR: FPU Overrun!\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void BTSSInterrupt(struct InterruptFrame* fr, unsigned code)            //interrupt 10
{
    kprintf("\nERROR: Bad TSS exception: Code=%x eip=%x\n", code, fr->eip);
    haltForever();
}

__attribute__((interrupt))
void NoSegInterrupt(struct InterruptFrame* fr, unsigned code)           //interrupt 11
{
    kprintf("\nERROR: No Segment exception: Code=%x eip=%x\n", code, fr->eip);
    haltForever();
}

__attribute__((interrupt))
void SFaultInterrupt(struct InterruptFrame* fr, unsigned code)         //interrupt 12
{
    kprintf("\nERROR: Stack Fault exception: Code=%x eip=%x\n", code, fr->eip);
    haltForever();
}

__attribute__((interrupt))
void GFaultInterrupt(struct InterruptFrame* fr, unsigned code)         //interrupt 13
{
    kprintf("\nERROR: General Fault exception: Code=%x eip=%x\n", code, fr->eip);
    haltForever();
}

__attribute__((interrupt))
void PFaultInterrupt(struct InterruptFrame* fr, unsigned code)         //interrupt 14
{
    unsigned faultingAddress;
    asm volatile( "mov eax,cr2" : "=a"(faultingAddress));
    kprintf("\nPage fault eip=%x addr=%x code=%x (%s %s)\n",
    fr->eip,
    faultingAddress, code,
    (code & 2) ? "write":"read",
    (code & 16) ? "instr":"data");
    haltForever();
}

__attribute__((interrupt))
void MFaultInterrupt(struct InterruptFrame* fr)                         //interrupt 16
{
    kprintf("\nERROR: Math Fault exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void MisalignedInterrupt(struct InterruptFrame* fr, unsigned code)      //interrupt 17
{
    kprintf("\nERROR: Misaligned exception: Code=%x eip=%x\n", code, fr->eip);
    haltForever();
}

__attribute__((interrupt))
void MCheckInterrupt(struct InterruptFrame* fr)                         //interrupt 18
{
    kprintf("\nERROR: Machine Check exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void SIMDInterrupt(struct InterruptFrame* fr)                           //interrupt 19
{
    kprintf("\nERROR: SIMD Fault exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void VMInterrupt(struct InterruptFrame* fr)                             //interrupt 20
{
    kprintf("\nERROR: VM Fault exception at eip=%x\n",fr->eip);
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
void timerInterrupt(struct InterruptFrame* fr)      //interrupt 32
{
    outb( 0x20, 32 );   //ack 1st PIC
    //logString("Timer Interrupt\n");
}

__attribute__((interrupt))
void KeyboardInterrupt(struct InterruptFrame* fr)   //interrupt 33
{
    unsigned keyCode = inb(0x60);   //get scancode
    logString("im getting keypress ");
    ksprintf(debugMsg,"ScanCode:%d\n\n",keyCode);
    logString(debugMsg);
    keyHandler(keyCode);
    outb( 0x20, 32 );               //ack 1st PIC
    
}

__attribute__((interrupt))
void CascadeInterrupt(struct InterruptFrame* fr)    //interrupt 34
{
    outb( 0x20, 32 );   //ack 1st PIC
}

__attribute__((interrupt))
void S2Interrupt(struct InterruptFrame* fr)         //interrupt 35
{
    outb( 0x20, 32 );   //ack 1st PIC
}

__attribute__((interrupt))
void S1Interrupt(struct InterruptFrame* fr)         //interrupt 36
{
    outb( 0x20, 32 );   //ack 1st PIC
}

__attribute__((interrupt))
void Av0Interrupt(struct InterruptFrame* fr)        //interrupt 37
{
    outb( 0x20, 32 );   //ack 1st PIC
}

__attribute__((interrupt))
void FloppyInterrupt(struct InterruptFrame* fr)     //interrupt 38
{
    outb( 0x20, 32 );   //ack 1st PIC
}

__attribute__((interrupt))
void ParPortInterrupt(struct InterruptFrame* fr)    //interrupt 39
{
    outb( 0x20, 32 );   //ack 1st PIC
}

__attribute__((interrupt))
void int40trap(struct InterruptFrame* fr)           //interrupt 40 RTC
{
    outb(0x70, 0xc);    //ack reading status reg
    inb(0x71);          //discard val
    jiffies++;
    outb( 0x20, 32 );   //ack 1st PIC
    outb( 0xa0, 32 );   //ack 2nd PIC
}

__attribute__((interrupt))
void VidInterrupt(struct InterruptFrame* fr)        //interrupt 41
{
    outb( 0x20, 32 );   //ack 1st PIC
    outb( 0xa0, 32 );   //ack 2nd PIC
}

__attribute__((interrupt))
void Av1Interrupt(struct InterruptFrame* fr)        //interrupt 42
{
    outb( 0x20, 32 );   //ack 1st PIC
    outb( 0xa0, 32 );   //ack 2nd PIC
}

__attribute__((interrupt))
void Av2Interrupt(struct InterruptFrame* fr)        //interrupt 43
{
    outb( 0x20, 32 );   //ack 1st PIC
    outb( 0xa0, 32 );   //ack 2nd PIC
}

__attribute__((interrupt))
void MouseInterrupt(struct InterruptFrame* fr)      //interrupt 44
{
    static unsigned ready = 0;
    int handler[4] = {SYSCALL_MOUSE_GET,0,0,0};
    for(ready = 1; ready < 4; ready++)
        handler[ready] = inb(0x60);
    syscall_handler(handler);
    outb( 0x20, 32 );   //ack 1st PIC
    outb( 0xa0, 32 );   //ack 2nd PIC
}

__attribute__((interrupt))
void FPUInterrupt(struct InterruptFrame* fr)        //interrupt 45
{
    outb( 0x20, 32 );   //ack 1st PIC
    outb( 0xa0, 32 );   //ack 2nd PIC
}

__attribute__((interrupt))
void DskC0Interrupt(struct InterruptFrame* fr)      //interrupt 46
{
    outb( 0x20, 32 );   //ack 1st PIC
    outb( 0xa0, 32 );   //ack 2nd PIC
}

__attribute__((interrupt))
void DskC1Interrupt(struct InterruptFrame* fr)      //interrupt 47
{
    outb( 0x20, 32 );   //ack 1st PIC
    outb( 0xa0, 32 );   //ack 2nd PIC
}

__attribute__((__interrupt__))
void syscallInterrupt(struct InterruptFrame* fr)
{
    if( fr->esp < 0x400000 || fr->esp > 0x800000-(4*4)) //Invalid parameter. Ignore the system call.
        return;
    int* espCheck = (int*)fr->esp;
    syscall_handler(espCheck);
    return;
}