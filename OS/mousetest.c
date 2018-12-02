
#include "syscalls.h"

#define SWAP(a,b) do{ int tmp=a; a=b; b=tmp; } while(0)

int abs(int x){
    if(x<0) return -x;
    else return x;
}


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

int write(int fd, char* buf, int size){
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

void setPixel(int x, int y){
    syscall(SYSCALL_SET_PIXEL, x,y, 0);
}

int sine(int degrees){
    //values[i] = sine(i) * 65536
    static int values[] = {0, 1143, 2287, 3429, 4571, 5711, 6850, 7986, 9120, 10252, 11380, 12504, 13625, 14742, 15854, 16961, 18064, 19160, 20251, 21336, 22414, 23486, 24550, 25606, 26655, 27696, 28729, 29752, 30767, 31772, 32767, 33753, 34728, 35693, 36647, 37589, 38521, 39440, 40347, 41243, 42125, 42995, 43852, 44695, 45525, 46340, 47142, 47929, 48702, 49460, 50203, 50931, 51643, 52339, 53019, 53683, 54331, 54963, 55577, 56175, 56755, 57319, 57864, 58393, 58903, 59395, 59870, 60326, 60763, 61183, 61583, 61965, 62328, 62672, 62997, 63302, 63589, 63856, 64103, 64331, 64540, 64729, 64898, 65047, 65176, 65286, 65376, 65446, 65496, 65526, 65536, 65526, 65496, 65446, 65376, 65286, 65176, 65047, 64898, 64729, 64540, 64331, 64103, 63856, 63589, 63302, 62997, 62672, 62328, 61965, 61583, 61183, 60763, 60326, 59870, 59395, 58903, 58393, 57864, 57319, 56755, 56175, 55577, 54963, 54331, 53683, 53019, 52339, 51643, 50931, 50203, 49460, 48702, 47929, 47142, 46340, 45525, 44695, 43852, 42995, 42125, 41243, 40347, 39440, 38521, 37589, 36647, 35693, 34728, 33753, 32767, 31772, 30767, 29752, 28729, 27696, 26655, 25606, 24550, 23486, 22414, 21336, 20251, 19160, 18064, 16961, 15854, 14742, 13625, 12504, 11380, 10252, 9120, 7986, 6850, 5711, 4571, 3429, 2287, 1143, 0, -1143, -2287, -3429, -4571, -5711, -6850, -7986, -9120, -10252, -11380, -12504, -13625, -14742, -15854, -16961, -18064, -19160, -20251, -21336, -22414, -23486, -24550, -25606, -26655, -27696, -28729, -29752, -30767, -31772, -32768, -33753, -34728, -35693, -36647, -37589, -38521, -39440, -40347, -41243, -42125, -42995, -43852, -44695, -45525, -46340, -47142, -47929, -48702, -49460, -50203, -50931, -51643, -52339, -53019, -53683, -54331, -54963, -55577, -56175, -56755, -57319, -57864, -58393, -58903, -59395, -59870, -60326, -60763, -61183, -61583, -61965, -62328, -62672, -62997, -63302, -63589, -63856, -64103, -64331, -64540, -64729, -64898, -65047, -65176, -65286, -65376, -65446, -65496, -65526, -65536, -65526, -65496, -65446, -65376, -65286, -65176, -65047, -64898, -64729, -64540, -64331, -64103, -63856, -63589, -63302, -62997, -62672, -62328, -61965, -61583, -61183, -60763, -60326, -59870, -59395, -58903, -58393, -57864, -57319, -56755, -56175, -55577, -54963, -54331, -53683, -53019, -52339, -51643, -50931, -50203, -49460, -48702, -47929, -47142, -46340, -45525, -44695, -43852, -42995, -42125, -41243, -40347, -39440, -38521, -37589, -36647, -35693, -34728, -33753, -32768, -31772, -30767, -29752, -28729, -27696, -26655, -25606, -24550, -23486, -22414, -21336, -20251, -19160, -18064, -16961, -15854, -14742, -13625, -12504, -11380, -10252, -9120, -7986, -6850, -5711, -4571, -3429, -2287, -1143};
    while(degrees > 360 )
        degrees -= 360;
    while(degrees < 0 )
        degrees += 360;
    return values[degrees];
}
    
void drawLine(int x1, int y1, int x2, int y2){
    if( x1 > x2 ){
        SWAP(x1,x2);
        SWAP(y1,y2);
    }
    
    
    //deltaX will always be nonnegative
    int deltaX = x2-x1;
    int deltaY = y2-y1;


    if( deltaX == 0 ){
        //vertical line
        int y;
        if(deltaY >= 0 ){
            for(y=y1;y<=y2;y++)
                setPixel(x1,y);
        }
        else{
            for(y=y1;y>=y2;y--)
                setPixel(x1,y);
        }
    }
    else if( deltaY == 0 ){
        //horizontal line
        int x;
        if(x2>x1){
            for(x=x1;x<=x2;x++)
                setPixel(x,y1);
        }
        else{
            for(x=x1;x<=x2;x--)
                setPixel(x,y1);
        }
    }
    else if( abs(deltaY) <= deltaX ){
        int dydx = (deltaY<<16)/(deltaX);
        int x;
        int y = y1 << 16;
        for(x=x1;x<=x2;x++){
            setPixel(x,y>>16);
            y += dydx;
        }
    }
    else{
        if( y1 > y2 ){
            SWAP(y1,y2);
            SWAP(x1,x2);
        }
        deltaX=-deltaX;
        deltaY=-deltaY;
        int dxdy = (deltaX<<16)/(deltaY);
        int y;
        int x = x1 << 16;
        for(y=y1;y<=y2;y++){
            setPixel(x>>16,y);
            x += dxdy;
        }
    }
        
}
        
//Bresenham's algorithm. Reference:
//https://www.cs.helsinki.fi/group/goa/mallinnus/lines/bresenh.html
void drawLineX(int x1, int y1, int x2, int y2){
    if( x1 > x2 ){
        SWAP(x1,x2);
        SWAP(y1,y2);
    }
    
    //deltaX will always be nonnegative
    int deltaX = x2-x1;
    int deltaY = y2-y1;

    if( deltaX == 0 ){
        //vertical line
        int y;
        if(deltaY >= 0 ){
            for(y=y1;y<=y2;y++)
                setPixel(x1,y);
        }
        else{
            for(y=y1;y>=y2;y--)
                setPixel(x1,y);
        }
    }
    else if( deltaY == 0 ){
        //horizontal line
        int x;
        if(x2>x1){
            for(x=x1;x<=x2;x++)
                setPixel(x,y1);
        }
        else{
            for(x=x1;x<=x2;x--)
                setPixel(x,y1);
        }
    }
    //~ else if( deltaX == deltaY ){
        //~ //45 degree / 
        //~ int x,y;
        //~ int n=deltaX;
        //~ for(x=x1,y=y1;n>0;x++,y++,n--)
            //~ setPixel(x,y);
    //~ }
    //~ else if( deltaX == -deltaY ){
        //~ //45 degree \ line 
        //~ int x,y;
        //~ int n=deltaX;
        //~ for(x=x1,y=y1;n>0;x++,y--,n--)
            //~ setPixel(x,y);
    //~ }
    else if( deltaY>0 ){
        if( deltaY <= deltaX ){
            //0...45 degrees / 
            int y = y1;
            int epsilon = 0;
            int x;
            for(x=x1;x<=x2;++x){
                setPixel(x,y);
                epsilon += deltaY;
                if( epsilon/2 > deltaX ){
                    y++;
                    epsilon -= deltaX;
                }
            }
        }
        else{
            //45...90 degrees
            int x = x1;
            int epsilon = 0;
            int y;
            for(y=y1;y<=y2;++y){
                setPixel(x,y);
                epsilon += deltaX;
                if( epsilon/2 > deltaY ){
                    x++;
                    epsilon -= deltaY;
                }
            }
        }
    } else if( deltaY < 0 ){
        deltaY=-deltaY;
        if( deltaY <= deltaX ){
            //0...45 degrees / 
            int y = y1;
            int epsilon = 0;
            int x;
            for(x=x1;x<=x2;++x){
                setPixel(x,y);
                epsilon += deltaY;
                if( epsilon/2 > deltaX ){
                    y--;
                    epsilon -= deltaX;
                }
            }
        }
        else{
            //45...90 degrees
            int x = x1;
            int epsilon = 0;
            int y;
            for(y=y1;y>=y2;--y){
                setPixel(x,y);
                epsilon += deltaX;
                if( epsilon/2 > deltaY ){
                    x++;
                    epsilon -= deltaY;
                }
            }
        }
    }
    //need to finish...
}
    
int main(){
    
    //we expect [0]=x, [1]=y, [2]=flags: 1=button 0 down; 2=button 1 down; 4=button 2 down
    int mouseStatus[4];
    int lastX=-1,lastY=-1;
    
    //~ drawLine(300,300, 600,300 );
    //~ drawLine(300,300, 300,100 );
    //~ drawLine(300,300, 100,300 );
    //~ drawLine(300,300, 300,500);
    //~ drawLine(300,300, 400,400);
    //~ drawLine(300,300, 400, 200);
    //~ drawLine(300,300, 200, 200);
    //~ drawLine(300,300, 200, 400);
    //~ drawLine(300,300, 500, 400);
    //~ drawLine(300,300, 500, 200);
    //~ drawLine(300,300, 100,200);
    //~ drawLine(300,300, 100,400);
    
    int deg;
    int radius = 200;
    int cx = 400;
    int cy = 300;
    for(deg=0;deg<360;deg += 5){
        int s = sine(deg);
        int c = sine(deg+90);
        int x = (s*radius) / 65536;
        int y = (c*radius) / 65536;
        drawLine(cx,cy,cx+x,cy+y);
    }
    
    while(1){
        //Syscall returns mouse x, y, and button flags
        mouseStatus[0] = SYSCALL_MOUSE_GET;
        syscallByReference(mouseStatus);
        if( mouseStatus[2] & 1 ){
            if( lastX != -1 ){
                drawLine(lastX,lastY,(int)mouseStatus[0],(int)mouseStatus[1]);
            }
        }
        lastX=(int)mouseStatus[0];
        lastY=(int)mouseStatus[1];

        if( mouseStatus[2] & 2 )
            syscall(SYSCALL_CLEAR,0,0,0);
    }
}
