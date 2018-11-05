#include "syscalls.h"
#include "errno.h"

int file_open(const char* fileName, int flags);
int file_close(int fd);
int file_read(int fd, void* buf, int count);
int file_write(int fd, const void* buf, int count);
int file_seek(int fd, int offset, int whence);

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

void syscall_handler(unsigned* ptr)
{
    int fd = ptr[1];
    unsigned buf = ptr[2];
    unsigned count = ptr[3];
    switch(ptr[0])
    {
        case SYSCALL_READ:
            if( buf < 0x400000 || buf > 0x800000 ||
                buf + count < 0x400000 || buf + count > 0x800000)
            {
                ptr[0] = -EFAULT;
                break;
            }
            ptr[0] = file_read(ptr[1], ptr[2], ptr[3]);
            ///do stuff
            break;
        case SYSCALL_WRITE:
            if( buf < 0x400000 || buf > 0x800000)
            {
                ptr [0] = -EFAULT;
                break;
            }
            if(count < 0 || buf+count >= 0x800000)
            {
                ptr[0] = -EINVAL;
                break;
            }
            ptr[0] = file_write(ptr[1], ptr[2], ptr[3]);
            break;
        case SYSCALL_OPEN:
            ptr[0] = file_open((char*)ptr[1], ptr[2]);  //ptr[1] : filename, prt[2] : flags
            break;
        case SYSCALL_CLOSE:
            ptr[0] = file_close(ptr[1]);                //ptr[1] : file descriptor
            break;
        case SYSCALL_EXIT:
            break;
        default:
            ptr[0] = -ENOSYS;
            break;
    }
}