/*
Code written by Thomas Gilman for Operating Systems One taught James Hudson.
Some functions and implementations of code use code from the OS slides,
the rest was written by Thomas Gilman.
*/
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
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0},{0,0,0,0}, {0,0,0,0}, {0,0,0}, {0,0,0}, {0,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {'\t',0,1,0}, {'`','~',1,0},{0,0,0,0}, {0,0,0,0}, {17,0,0,0}, {18,0,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {'q','Q',1,0}, {'1','!',1,0}, {0,0,0,0}, {0,0,0,0},{0,0,0,0}, {'z','Z',1,0}, {'s','S',1,0}, {'a','A',1,0}, {'w','W',1,0}},
    {{'2','@',1,0}, {0,0,0,0}, {0,0,0,0}, {'c','C',1,0}, {'x','X',1,0},{'d','D',1,0}, {'e','E',1,0}, {'4','$',1,0}, {'3','#',1,0}, {0,0,0,0}},
    {{0,0,0,0}, {' ',0,1,0}, {'v','V',1,0}, {'f','F',1,0}, {'t','T',1,0},{'r','R',1,0}, {'5','%',1,0}, {0,0,0,0}, {0,0,0,0}, {'n','N',1,0}},
    {{'b','B',1,0}, {'h','H',1,0}, {'g','G',1,0}, {'y','Y',1,0}, {'6','^',1,0},{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {'m','M',1,0}, {'j','J',1,0}},
    {{'u','U',1,0}, {'7','&',1,0}, {'8','*',1,0}, {0,0,0,0}, {0,0,0,0},{',','<',1,0}, {'k','K',1,0}, {'i','I',1,0}, {'o','O',1,0}, {'0',')',1,0}},
    {{'9','(',1,0}, {0,0,0,0}, {0,0,0,0}, {'.','>',1,0}, {'/','?',1,0},{'l','L',1,0}, {';',':',1,0}, {'p','P',1,0}, {'-','_',1,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {'\'','"',1,0}, {0,0,0,0}, {'[','{',1,0},{'=','+',1,0}, {0,0,0,0}, {0,0,0,0}, {88,0,0,0}, {18,0,0,0}},
    {{'\n',0,1,0}, {']','}',1,0}, {0,0,0,0}, {'\\','|',1,0}, {0,0,0,0},{0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {127,0,1,0}, {0,0,0,0}, {0,0,0,0},{'1','!',1,0}, {0,0,0,0}, {'4',0,1,0}, {'7',0,1,0}, {0,0,0,0}},
    {{0,0,0,0}, {0,0,0,0}, {'0',0,1,0}, {'.',0,1,0}, {'2',0,1,0},{'5',0,1,0}, {'6',0,1,0}, {'8',0,1,0}, {0,0,0,0}, {0,0,0,0}},
    {{0,0,0,0}, {'+',0,1,0}, {'3',0,1,0}, {'-',0,1,0}, {0,0,0,0},{'9',0,1,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}, {0,0,0,0}}
};

unsigned ring0StackInfo[] =
{
    0,
    (unsigned)(kernelStack+sizeof(kernelStack)),
    1<<3
};

struct GDTEntry gdt[] = 
{
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
    unsigned numCpy;
    while(!linebuf_ready)
    {
        sti();                  //enable interrupts
        haltUntilInterrupt();
    }
    if(buffer >= (char*)0x400000 && (buffer + num) < (char*)0x800000)
    {
        numCpy = (num < linebuf_chars) ? num : linebuf_chars;
        kmemcpy(buffer, linebuf, numCpy);
        kmemset(linebuf, numCpy);
        linebuf_chars = 0;
        linebuf_ready = 0;
        return numCpy;
    }
    return 0;
}

void keyHandler(unsigned keyValIn)
{
    unsigned col = keyValIn % 10;
    unsigned row = (keyValIn - col) / 10;
    static unsigned keyReleased = 0, shifted = 0, capsLock = 0, capCount = 0;
    struct ScanCode *k = &keyTable[row][col];
    
    if(keyValIn != 0xf0)
    {
        if(k->keyPressed == 0 && k->printable)                              //printable char
        {
            if(k->lowerVal == 127 && linebuf_chars > 0)                     //backspace
            {
                console_putc(k->lowerVal);
                linebuf[--linebuf_chars] = 0;
            }
            else if(k->lowerVal == '\n')                                    //newline
            {
                console_putc(k->lowerVal);
                linebuf_ready = 1;
            }
            else if(linebuf_chars < LINEBUF_SIZE && k->lowerVal != 127)     //put char
            {
                if((shifted && !capsLock) || (!shifted && capsLock))
                {
                    console_putc(k->upperVal);
                    linebuf[linebuf_chars++] = k->upperVal;
                }
                else
                {
                    console_putc(k->lowerVal);
                    linebuf[linebuf_chars++] = k->lowerVal;
                }
            }
            sleep(100);
            k->keyPressed = 1;
        }
        else if(k->lowerVal == 18 && k->keyPressed == 0)                    //shift
        {
            shifted = 1;
            k->keyPressed = 1;
        }
        else if(k->lowerVal == 88 && k->keyPressed == 0)                    //capslock
        {
            capCount = 1;
            capsLock = 1;
            k->keyPressed = 1;
        }
        else if(keyReleased == 1 && k->keyPressed == 1)                     //reset key
        {
            if(k->lowerVal == 88 && capCount != 0)                          //capslock turnoff counter
            {
                capCount--;
            }
            else
            {
                k->keyPressed = 0;
                keyReleased = 0;
                if(k->lowerVal == 18)   //shift
                    shifted = 0;
                if(k->lowerVal == 88)   //capsLock
                    capsLock = 0;
            }
        }
    }
    else if(keyValIn == 0xf0)
    {
        keyReleased = 1;
    } 
}

// static void send(unsigned short port, unsigned char val)
// {
//     while(inb(0x64) & 2){;}
//     outb(port, val);
// }

// static unsigned char recv()
// {
//     while(!(inb(0x64) & 1)){;}
//     return inb(0x60);
// }

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

void setupKeyBoard(void)
{
    outb(0x64,0x20);            //command: Read config bits
    unsigned oldv = inb(0x60);  //get current config
    oldv &= ~0x40;              //mask bit 6: Turn off translation
    outb(0x64,0x60);            //command: Write config bits
    outb(0x60,oldv);            //The new value
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
                ptr[0] = keyboard_getline((char*)ptr[2], count);
            }
            else if(fd < 3) //illegal to read from screen
            {
                ptr[0] = -ENOTTY;
                logString("ERROR: cant read from screen!!\n");
                break;
            }
            else
            {
                pass = file_read(fd, (char*)ptr[2], count);
                ptr[0] = pass;
            }
            if(pass < 0)
            {
                ksprintf(debugMsg,"ERROR: failed to write to fd:%d, code:%d\n!!", fd, ptr[0]);
                logString(debugMsg);
            }
            else logString("read from file\n");
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
            logString("playing sound...\n");
            playSound(ptr[1]);
            break;
        case SYSCALL_SLEEP: ///FIX HERE
            logString("waiting...\n");
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
    setupKeyBoard();
    struct LIDT lidt;
    lidt.size = sizeof(idt);
    lidt.addr = &idt[0];
    asm volatile("lidt [eax]" : : "a"(&lidt) : "memory" );
    asm volatile("sti" : : : "memory"); //interrupt inhibit
    return SUCCESS;
}