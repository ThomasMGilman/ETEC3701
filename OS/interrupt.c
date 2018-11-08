#include "interrupt.h"
#include "syscalls.h"
#include "file.h"
#include "kprintf.h"

#define INTERRUPT_SIZE 49

//UTIL 
void outb(unsigned short port, unsigned char value);
unsigned char inb(unsigned short port);
void haltForever(void);
void logString(char* myString);

char debugMsg[100];
volatile unsigned jiffies = 0;
unsigned Frequency = 2;

unsigned ring0StackInfo[] =
{
    0,
    (unsigned)(kernelStack+sizeof(kernelStack)),
    1<<3
};

struct GDTEntry gdt[] = {
    { 0,0,0,0,0,0 },            //zeros
    { 0xffff, 0,0,0, 0xcf92, 0},//data, ring 0
    { 0xffff, 0,0,0, 0xcf9a, 0},//code, ring 0
    { 0xffff, 0,0,0, 0xcff2, 0},//data, ring 3
    { 0xffff, 0,0,0, 0xcffa, 0},//code, ring 3
    { 0,0,0,0,0,0 }             //task selector
};

struct IDTEntry idt[INTERRUPT_SIZE];

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
    unsigned powLoop = 15 - rate;
    if(powLoop == 0) Frequency = 2;
    else
    {
        while(powLoop--)
        {
            Frequency *= 2;
        }
    }
    ksprintf(debugMsg,"rate:%d Freq:%d\n", rate, Frequency);
    logString(debugMsg);
}

__attribute__((interrupt))
void divideByZeroInterrupt(struct InterruptFrame* fr)   //interrupt 0
{
    kprintf("\nERROR: Division by Zero is undefined!\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void debugTrapInterrupt(struct InterruptFrame* fr)      //interrupt 1
{
    kprintf("\nERROR: Debug Trap!\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void NMIInterrupt(struct InterruptFrame* fr)            //interrupt 2
{
    kprintf("\nERROR: NMI!!\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void int3Interrupt(struct InterruptFrame* fr)           //interrupt 3
{
    kprintf("\nERROR: int3 Trap!\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void OverflowInterrupt(struct InterruptFrame* fr)       //interrupt 4
{
    kprintf("\nERROR: Overflow!!\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void BCInterrupt(struct InterruptFrame* fr)             //interrupt 5
{
    kprintf("\nERROR: BoundCheck!!\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void badOpcodeInterrupt(struct InterruptFrame* fr)      //interrupt 6
{
    kprintf("\nERROR: Bad opcode!\nFatal exception at eip=%x\n",fr->eip);
    haltForever();
}

__attribute__((interrupt))
void NFPUInterrupt(struct InterruptFrame* fr)           //interrupt 7
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
    kprintf("\nERROR: Page Fault exception: Code=%x eip=%x\n", code, fr->eip);
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
    outb(0x64,0x20);            //command: Read config bits
    unsigned oldv = inb(0x60);  //get current config
    oldv &= ~0x40;              //mask bit 6: Turn off translation
    outb(0x64,0x60);            //command: Write config bits
    outb(0x60,oldv);            //The new value
    outb( 0x20, 32 );           //ack 1st PIC
    keyHandler(oldv);
}

__attribute__((interrupt))
void CascadeInterrupt(struct InterruptFrame* fr)    //interrupt 34
{
    outb( 0x20, 32 );   //ack 1st PIC
}

__attribute__((interrupt))
void S2Interrupt(struct InterruptFrame* fr)      //interrupt 35
{
    outb( 0x20, 32 );   //ack 1st PIC
}

__attribute__((interrupt))
void S1Interrupt(struct InterruptFrame* fr)      //interrupt 36
{
    outb( 0x20, 32 );   //ack 1st PIC
}

__attribute__((interrupt))
void Av0Interrupt(struct InterruptFrame* fr)    //interrupt 37
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
    outb( 0x20, 32 );   //ack 1st PIC
    outb( 0xa0, 32 );   //ack 2nd PIC
    jiffies++;
}

__attribute__((interrupt))
void VidInterrupt(struct InterruptFrame* fr)  //interrupt 41
{
    outb( 0x20, 32 );   //ack 1st PIC
    outb( 0xa0, 32 );   //ack 2nd PIC
}

__attribute__((interrupt))
void Av1Interrupt(struct InterruptFrame* fr)  //interrupt 42
{
    outb( 0x20, 32 );   //ack 1st PIC
    outb( 0xa0, 32 );   //ack 2nd PIC
}

__attribute__((interrupt))
void Av2Interrupt(struct InterruptFrame* fr)  //interrupt 43
{
    outb( 0x20, 32 );   //ack 1st PIC
    outb( 0xa0, 32 );   //ack 2nd PIC
}

__attribute__((interrupt))
void MouseInterrupt(struct InterruptFrame* fr) //interrupt 44
{
    outb( 0x20, 32 );   //ack 1st PIC
    outb( 0xa0, 32 );   //ack 2nd PIC
}

__attribute__((interrupt))
void FPUInterrupt(struct InterruptFrame* fr) //interrupt 45
{
    outb( 0x20, 32 );   //ack 1st PIC
    outb( 0xa0, 32 );   //ack 2nd PIC
}

__attribute__((interrupt))
void DskC0Interrupt(struct InterruptFrame* fr) //interrupt 46
{
    outb( 0x20, 32 );   //ack 1st PIC
    outb( 0xa0, 32 );   //ack 2nd PIC
}

__attribute__((interrupt))
void DskC1Interrupt(struct InterruptFrame* fr) //interrupt 47
{
    outb( 0x20, 32 );   //ack 1st PIC
    outb( 0xa0, 32 );   //ack 2nd PIC
}

__attribute__((__interrupt__))
void syscallInterrupt(struct InterruptFrame* fr)
{
    if( fr->esp < 0x400000 || fr->esp > 0x800000-(4*4)) //Invalid parameter. Ignore the system call.
        return;
    unsigned* espCheck = (unsigned*)fr->esp;
    syscall_handler(espCheck);
    return;
}

void keyHandler(unsigned keyValIn)
{
    ;
}

void table(int i, void* func){
    unsigned x = (unsigned)func;
    idt[i].addrLow = x&0xffff;
    idt[i].selector = 2 << 3;
    idt[i].zero = 0;
    idt[i].flags = 0x8e;
    idt[i].addrHigh = x>>16;
}

void setInterruptTable(void)
{
    unsigned index;
    for(index = 0; index < INTERRUPT_SIZE; index++)
    {
        if(index == 0)
            table(index, divideByZeroInterrupt);
        else if(index == 1)
            table(index, debugTrapInterrupt);
        else if(index == 2)
            table(index, NMIInterrupt);
        else if( index == 3)
            table(index, int3Interrupt);
        else if( index == 4)
            table(index, OverflowInterrupt);
        else if( index == 5)
            table(index, BCInterrupt);
        else if(index == 6)
            table(index, badOpcodeInterrupt);
        else if( index == 7)
            table(index, NFPUInterrupt);
        else if( index == 8)
            table(index, DFInterrupt);
        else if( index == 9)
            table(index, FPUOInterrupt);
        else if( index == 10)
            table(index, BTSSInterrupt);
        else if( index == 11)
            table(index, NoSegInterrupt);
        else if( index == 12)
            table(index, SFaultInterrupt);
        else if( index == 13)
            table(index, GFaultInterrupt);
        else if( index == 14)
            table(index, PFaultInterrupt);
        else if( index == 16)
            table(index, MFaultInterrupt);
        else if( index == 17)
            table(index, MisalignedInterrupt);
        else if( index == 18)
            table(index, MCheckInterrupt);
        else if( index == 19)
            table(index, SIMDInterrupt);
        else if( index == 20)
            table(index, VMInterrupt);
        else if( index == 32)
            table(index, timerInterrupt);
        else if( index == 33)
            table(index, KeyboardInterrupt);
        else if( index == 34)
            table(index, CascadeInterrupt);
        else if( index == 35)
            table(index, S1Interrupt);
        else if( index == 36)
            table(index, S2Interrupt);
        else if( index == 37)
            table(index, Av0Interrupt);
        else if( index == 38)
            table(index, FloppyInterrupt);
        else if( index == 39)
            table(index, ParPortInterrupt);
        else if( index == 40)
            table(index, int40trap);
        else if( index == 41)
            table(index, VidInterrupt);
        else if( index == 42)
            table(index, Av1Interrupt);
        else if( index == 43)
            table(index, Av2Interrupt);
        else if( index == 44)
            table(index, MouseInterrupt);
        else if ( index == 45)
            table(index, FPUInterrupt);
        else if( index == 46)
            table(index, DskC0Interrupt);
        else if( index == 47)
            table(index, DskC1Interrupt);
        else if(index == 48)
        {
            table(index, syscallInterrupt);
            idt[48].flags = 0xee;           //accessible to ring 3/user
        }
        else
            table(index, unknownInterrupt);
    }
}

void setupGDT(void)
{
    unsigned tmp = (unsigned)ring0StackInfo;
    gdt[5].limitLow = sizeof(ring0StackInfo);
    gdt[5].base0 = tmp & 0xff;
    gdt[5].base1 = (tmp>>8) & 0xff;
    gdt[5].base2 = (tmp>>16) & 0xff;
    gdt[5].flagsAndLimitHigh = 0x00e9;
    gdt[5].base3 = (tmp>>24) & 0xff;
}

void syscall_handler(unsigned* ptr)
{
    signed pass = 0;
    int fd = ptr[1];
    unsigned buf = ptr[2];
    unsigned count = ptr[3];
    int divisor;
    short v;
    unsigned timeToWait;
    switch(ptr[0])
    {
        case SYSCALL_READ:
            if( (buf < 0x400000 || buf > 0x800000) ||
                (buf + count < 0x400000 || buf + count > 0x800000))
            {
                ptr[0] = -EFAULT;
                logString("ERROR: cant read from outside of userspace!!\n");
                break;
            }
            else if(fd == 1 || fd == 2) //illegal to read from screen
            {
                ptr[0] = -ENOTTY;
                logString("ERROR: cant read from screen!!\n");
                break;
            }
            pass = file_read(fd, (char*)ptr[2], count);
            ptr[0] = pass;
            if(pass >= 0) logString("successfully read from file!!\n");
            else
            {
                ksprintf(debugMsg,"ERROR: failed to write to fd:%d, code:%d\n!!", fd, ptr[0]);
                logString(debugMsg);
            }
            break;
        case SYSCALL_WRITE:
            if( (buf < 0x400000 || buf > 0x800000) ||
                (count < 0 || buf+count >= 0x800000))
            {
                ptr [0] = -EFAULT;
                logString("ERROR: cant write outside of userspace!!\n");
                break;
            }
            else if(fd == 0)            //illegal to write to keyboard
            {
                ptr[0] = -ENOTTY;
                logString("ERROR: cant write to keyboard!!\n");
                break;
            }
            pass = file_write(fd, (char*)ptr[2], count);
            ptr[0] = pass;
            if(pass < 0)
            {
                ksprintf(debugMsg,"ERROR: failed to write to fd:%d, code:%d!!\n", fd, ptr[0]);
                logString(debugMsg);
            }
            else logString("wrote to file\n");
            break;
        case SYSCALL_OPEN:
            ptr[0] = file_open((const char*)ptr[1], ptr[2]);//ptr[1] : filename, prt[2] : flags
            if(ptr[0] >= 0)
            {
                ksprintf(debugMsg,"fd:%d File:%s opened successfully!!\n",ptr[0] ,(const char*)ptr[1]);
                logString(debugMsg);
            }
            break;
        case SYSCALL_CLOSE:
            pass = file_close(fd);
            if(pass == 0) logString("file closed!!\n");
            else logString("No such file to close!!\n");
            ptr[0] = pass;
            break;
        case SYSCALL_EXIT:
            break;
        case SYSCALL_HALT:
            //logString("Halting");
            asm volatile(
                "sti\n"
                "hlt":::"memory");
            break;
        case SYSCALL_PLAY:
            //logString("playing\n");
            if(ptr[1] != 0)
            {
                divisor  = 1193180 / ptr[1];                //ptr[1] : Frequency
                ksprintf(debugMsg,"div:%d, numPass:%d\n", divisor, ptr[1]);
                logString(debugMsg);
            }
            else
            {
                logString("div 0\n");
                divisor = 0;
            }
            outb(0x42, 0xb6);
            outb(0x42, (const unsigned)(divisor & 0xff));   //low byte
            outb(0x42, (const unsigned)(divisor & 0xff00)); //high byte
            v = inb(0x61);
            ksprintf(debugMsg,"CurDiv:%d, Freq:%d, v:%d\n",divisor, ptr[1], v);
            logString(debugMsg);
            if(v & 0x0003)
                outb(0x61, (v|3));
            logString("\n");
            break;
        case SYSCALL_SLEEP:
            //logString("waiting\n");
            timeToWait = ptr[1] * (1/Frequency) * 10000;        //wait time
            if(jiffies == timeToWait)
            {
                logString("done waiting\n");
                v = inb(0x61);
                outb(0x61, (v & 0xfffc));                       //turn off the lower two bits
                jiffies = 0;
            }
            break;
        case SYSCALL_LOG:
            outb(0x3f8, (char)ptr[1]);
            break;
        default:
            ptr[0] = -ENOSYS;
            break;
    }
}

int exec(const char* filename)
{   
    unsigned fd, bytes;
    if((fd = file_open(filename,0)) < 0)
    {
        ksprintf(debugMsg,"FileName:%s failed to open!! failed to execute!!\n",filename);
        logString(debugMsg);
        return fd;
    }
    bytes = file_read(fd, (void *)0x400000, file_table[fd].ino.size);
    ksprintf(debugMsg,"numRead:%d\n",bytes);
    logString(debugMsg);

    if((fd = file_close(fd)) < 0)
    {
        ksprintf(debugMsg,"FileName:%s failed to close!! failed to execute!!\n",filename);
        logString(debugMsg);
        return fd;
    }
    asm volatile(
        "mov ax,27\n"
        "mov ds,ax\n"
        "mov es,ax\n"
        "mov fs,ax\n"
        "mov gs,ax\n"
        "push 27\n"
        "push 0x800000\n"
        "pushf\n"       //push eflags register
        "push 35\n"
        "push 0x400000\n"
        "iret"
        ::: "eax","memory" );
    kprintf("We should never get here!\n");
    haltForever();
    return -1;
}

int interrupt_init(void)
{
    struct LGDT lgdt;
    setupPICS_RTC(6);
    setupGDT();
    lgdt.size = sizeof(gdt);
    lgdt.addr = &gdt[0];
    asm volatile( "lgdt [eax]\n"
            "ltr bx"
        :                   //no outputs
        :   "a"(&lgdt),     //put address of gdt in eax
            "b"((5<<3)|3)   //put task register index in ebx
        : "memory" );
    setInterruptTable();
    struct LIDT lidt;
    lidt.size = sizeof(idt);
    lidt.addr = &idt[0];
    asm volatile("lidt [eax]" : : "a"(&lidt) : "memory" );
    asm volatile("sti" : : : "memory"); //interrupt inhibit
    return SUCCESS;
}