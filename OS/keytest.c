
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
    return syscall( SYSCALL_OPEN, (unsigned)filename, 0, 0);
}

int read(int fd, char* buf, int size){
    return syscall(SYSCALL_READ,fd,(unsigned)buf,size);
}

int write(int fd, char* buf, int size){
    return syscall(SYSCALL_WRITE,fd,(unsigned)buf,size);
}

int log(char c)
{
    return syscall(SYSCALL_LOG, c, 0, 0);
}
    
char* itoc(unsigned value, int width, char* buf){
    int places;
    char* p=buf;
    for(places=0;places < width || value>0;places++){
         *p = value%10 + '0';
         value  = value/10;
         p++;
    }
    *p = 0;
    char* rv = p;
    p--;
    char* q = buf;
    while(p>q){
        char tmp = *p;
        *p = *q;
        *q = tmp;
        p--;
        q++;
    }
    return rv;
}

const char msg = 'H';
    
int main(){
    char buf[128];
    char numbuff[5];
    while(1){
        write(1,"--> ",4);
        log(msg);
        int nr = read(0,buf,sizeof(buf));
        log(msg);
        write(1,"You typed ",10);
        itoc(nr,sizeof(numbuff)-1,numbuff);
        write(1,numbuff,sizeof(numbuff)-1);
        log(msg);
        write(1," characters: ",13);
        write(1,buf,nr);
        write(1,"\n",1);
        log(msg);
    }
}
