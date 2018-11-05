#include "syscalls.h"

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

