
#include "syscalls.h"

int syscallByReference(int A[]){
    asm volatile(
        "push edx\n"
        "push ecx\n"
        "push ebx\n"
        "push eax\n"
        "int 48\n"
        "pop eax\n"
        "pop ebx\n"
        "pop ecx\n"
        "pop edx\n"
        : "+a"(A[0]), "+b"(A[1]), "+c"(A[2]),"+d"(A[3])
    );
    return A[0];
}


int syscall(int p0, int p1, int p2, int p3){
    int tmp[] = {p0,p1,p2,p3};
    syscallByReference(tmp);
    return tmp[0];
}

int open(const char* filename){
    return syscall( SYSCALL_OPEN, (unsigned)filename, 0, 0);
}

int read(int fd, char* buf, int size){
    return syscall(SYSCALL_READ,fd,(unsigned)buf,size);
}

int write(int fd, const char* buf, int size){
    return syscall(SYSCALL_WRITE,fd,(unsigned)buf,size);
}


unsigned div(unsigned numerator, unsigned denominator){
    return numerator/denominator;
}
  
unsigned mod(unsigned numerator, unsigned denominator){
    return numerator % denominator;
}

char* itoc(unsigned value, int width, char* buf){
    int places;
    char* p=buf;
    for(places=0;places < width || value>0;places++){
         *p = mod(value,10) + '0';
         value  = div(value,10);
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
    
int mystrlen(const char* x){
    int n=0;
    while(*x++)
        n++;
    return n;
}

const char instr[] = {"Type 'k' 'd' 'h' or 'x' to fault --> "};
typedef void (*FUNC)(int);

int main(){
    char buf[128];
    char numbuff[5];
    int le = mystrlen(instr);
    while(1){
        write(1,instr,le);
        int nr = read(0,buf,sizeof(buf));
        write(1,"You typed ",10);
        itoc(nr,sizeof(numbuff)-1,numbuff);
        write(1,numbuff,sizeof(numbuff)-1);
        write(1," characters: ",13);
        write(1,buf,nr);
        write(1,"\n",1);
        if( nr == 1 && buf[0] == 'k' ){
            //write kernel memory
            char* p = (char*) 0x00100000;
            *p = 42;
        }
        else if( nr == 1 && buf[0] == 'd' ){
            //read device memory
            char* p = (char*) 0x90000000;
            int y = *p;
            if( y == 0 ){}
        }
        else if( nr == 1 && buf[0] == 'h' ){
            //write high memory
            char* p = (char*) 0x00c00000;
            *p = 42;
        }
        else if( nr == 1 && buf[0] == 'x' ){
            //execute kernel code
            FUNC p = (FUNC) 0x00104000;
            p(52);
        }
    }
}
