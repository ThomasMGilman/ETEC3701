#include "sys.h"
#include "file.h"
#include "kprintf.h"
#include "syscalls.h"
#include "errno.h"

void logString(char* myString);
void haltForever(void);

char debugMsg[100];

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
    // for(int i = 0; i < 4096; i++) //Hexdump file to log
    // {
    //     ksprintf(debugMsg,"%.2X ",((char*)0x400000)[i]&0xff);
    //     logString(debugMsg);
    // }

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