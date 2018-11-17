#include "interrupt.h"
#include "intertable.h"
#include "syscalls.h"
#include "file.h"
#include "util.h"
#include "kprintf.h"

#define INTERRUPT_SIZE 49
#define LINEBUF_SIZE 20

void console_putc(char c);

static char linebuf[LINEBUF_SIZE];
static int linebuf_chars = 0;
static volatile int linebuf_ready=0;
volatile unsigned jiffies = 0;
unsigned Frequency = 2;

char debugMsg[100];

unsigned throwAway = 1;

struct ScanCode keyTable[13][10] = { 
    {{0,0}, {0,0}, {0,0}, {0,0}, {0,0},{0,0}, {0,0}, {0,0}, {0,0}, {0,0}},
    {{0,0}, {0,0}, {0,0}, {9,1}, {39,1},{0,0}, {0,0}, {0,0}, {0,0}, {0,0}},
    {{0,0}, {113,1}, {49,1}, {0,0}, {0,0},{0,0}, {122,1}, {115,1}, {97,1}, {119,1}},
    {{50,1}, {0,0}, {0,0}, {99,1}, {120,1},{100,1}, {101,1}, {52,1}, {51,1}, {0,0}},
    {{0,0}, {32,1}, {118,1}, {102,1}, {116,1},{114,1}, {53,1}, {0,0}, {0,0}, {110,1}},
    {{98,1}, {104,1}, {103,1}, {121,1}, {54,1},{0,0}, {0,0}, {0,0}, {109,1}, {106,1}},
    {{117,1}, {55,1}, {56,1}, {0,0}, {0,0},{44,1}, {107,1}, {105,1}, {111,1}, {48,1}},
    {{57,1}, {0,0}, {0,0}, {46,1}, {47,1},{108,1}, {59,1}, {112,1}, {45,1}, {0,0}},
    {{0,0}, {0,0}, {96,1}, {0,0}, {91,1},{61,1}, {0,0}, {0,0}, {0,0}, {0,0}},
    {{10,1}, {93,1}, {0,0}, {92,1}, {0,0},{0,0}, {0,0}, {0,0}, {0,0}, {0,0}},
    {{0,0}, {0,0}, {127,1}, {0,0}, {0,0},{49,1}, {0,0}, {52,1}, {55,1}, {0,0}},
    {{0,0}, {0,0}, {48,1}, {46,1}, {50,1},{53,1}, {54,1}, {56,1}, {0,0}, {0,0}},
    {{0,0}, {43,1}, {51,1}, {45,1}, {0,0},{57,1}, {0,0}, {0,0}, {0,0}, {0,0}}
};

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

//&nbsp
int keyboard_getline(char* buffer, unsigned num)
{
    unsigned numCpy = (num < (LINEBUF_SIZE - linebuf_chars)) ? num : (LINEBUF_SIZE - linebuf_chars);
    while(!linebuf_ready)
    {
        sti();                  //enable interrupts
        haltUntilInterrupt();
    }
    if((*buffer) >= 0x400000 && ((*buffer)+num) < 0x800000)
    {
        logString("here\n");
        kmemcpy(linebuf + linebuf_chars, buffer, numCpy);
        linebuf_chars += numCpy;
        linebuf_ready = 0;
        return numCpy;
    }
    return 0;
}

void keyHandler(unsigned keyValIn)
{
    unsigned col = keyValIn % 10;
    unsigned row = (keyValIn - col) / 10;
    struct ScanCode k = keyTable[row][col];

    if(throwAway != 0)
    {
        throwAway--;
    }    
    else
    {
        if(k.printable)
        {
            if(k.keyVal == 127 && linebuf_chars > 0)
            {
                linebuf[linebuf_chars] = 0;
                --linebuf_chars;
                console_putc(k.keyVal);
            }
            else if(k.keyVal == '\n')
            {
                linebuf_ready = 1;
                console_putc(k.keyVal);
            }
            else if(linebuf_chars < LINEBUF_SIZE)
            {
                
                ksprintf(debugMsg,"keyIN:%d, charDec:%d\n charIn:%c\n",keyValIn, k.keyVal, k.keyVal);
                logString(debugMsg);
                linebuf[linebuf_chars++] = k.keyVal;
                console_putc(k.keyVal);
            }
        }
        ksprintf(debugMsg,"keyIN:%d\n", keyValIn);
        logString(debugMsg);
        sleep(100);
        throwAway++;
    }
    ksprintf(debugMsg,"keyIN:%d\n", keyValIn);
    logString(debugMsg);
}

static void send(unsigned short port, unsigned char val)
{
    while(inb(0x64) & 2){;}
    outb(port, val);
}

static unsigned char recv()
{
    while(!(inb(0x64) & 1)){;}
    return inb(0x60);
}

void table(int i, void* func)
{
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
            else if(fd == 0)
            {
                ptr[0] = keyboard_getline((char*)ptr[2], ptr[3]);
            }
            else if(fd < 3) //illegal to read from screen
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
            logString("here");
            playSound(ptr[1]);
            break;
        case SYSCALL_SLEEP: ///FIX HERE
            logString("waiting\n");
            sleep(ptr[1]);
            break;
        case SYSCALL_LOG:
            outb(QEMUPORT,ptr[1]);
            logString("\n");
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

/*
    RATE_VALUES:
    15=2Hz, 14=4Hz, 13=8Hz, 12=16Hz, 11=32Hz, 10=64Hz, 9=128Hz, 
    8=256Hz, 7=512Hz, 6=1024Hz, 5=2048Hz, 4=4096Hz, 3=8192Hz
*/
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