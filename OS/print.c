#include "syscalls.h"

int syscall(int p0, int p1, int p2, int p3){
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

int open(const char* filename){
    return syscall(SYSCALL_OPEN,(unsigned)filename, 0 , 0);
}

int read(int fd, char* buf, int size){
    return syscall(SYSCALL_READ,fd,(unsigned)buf,size);
}

int write(int fd, char* buf, int size){
    return syscall(SYSCALL_WRITE,fd,(unsigned)buf,size);
}

int main(int argc, char* argv[])
{
    char buf[32];
    int fd = open("article4.txt");
    if( fd >= 0 ){
        while(1){
            int rv = read(fd,buf,sizeof(buf));
            if( rv <= 0 )
                break;
            write(1,buf,rv);
        }
    }
    while(1){
    }
    return 0;
    
}

