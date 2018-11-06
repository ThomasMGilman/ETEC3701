#include "syscalls.h"

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

int open(const char* filename){
    return syscall(SYSCALL_OPEN, (unsigned)filename, 0 , 0);
}

int read(int fd, char* buf, int size){
    return syscall(SYSCALL_READ, fd, (unsigned)buf, size);
}

int write(int fd, char* buf, int size){
    return syscall(SYSCALL_WRITE, fd, (unsigned)buf, size);
}
int close(int fd){
    return syscall(SYSCALL_CLOSE, fd, 0, 0);
}
void wait(void)
{
    return syscall(SYSCALL_HALT,0,0,0);
}

int main(int argc, char* argv[])
{
    char buf[32];
    int fd = open("article4.txt");
    int test = 1;
    if( fd >= 0 ){
        while(test--){
            int rv = read(fd,buf,sizeof(buf));
            if( rv <= 0 )
                break;
            write(1,buf,rv);
        }
        close(fd);
    }
    register unsigned flag asm("esi");
    flag=0x31337;
    wait();
    return 0;
} 