#include "kprintf.h"

#define SUCCESS 0

extern int file_open(const char* fname, int flags);
extern int file_read(int fd, void* buf, int num);
extern int file_close(int fd);
extern int file_seek(int fd, int offset, int whence);
#define SEEK_SET 0

static int readline(int fd, char buff[]){
    int rv;
    char* p;
    p=buff;
    int n=0;
    while(1){
        n++;
        if( n > 64){
            kprintf("Runaway readline!");
            return -1;
        }
        rv = file_read(fd,p,1);
        if( rv == 0 ){
            *p=0;
            return 0;
        }
        else if( rv == 1 ){
            if( *p == '\r' ){
            }
            else if( *p == '\n'){
                *p = 0;
                return SUCCESS;
            }
            else{
                p++;
            }
        }
        else{
            kprintf("Error in readline\n");
            return -1;
        }
    }
}
            
static int atoi(char* p ){
    int value=0;
    while(*p){
        value *= 10;
        value += *p - '0';
        p++;
    }
    return value;
}

static int readFully(int fd,void* vp, int count){
    char* p = (char*) vp;
    int rv;
    while(count > 0 ){
        rv = file_read(fd,p,count);
        if( rv <= 0 ){
            kprintf("readFully: error: %d\n",rv);
            return -1;
        }
        p += rv;
        count -= rv;
    }
    return 0;
}
    
int displayPPM(int fd, void* fbv, 
    int redshift, int redmask, int greenshift, int greenmask,
    int blueshift, int bluemask, int bpp,
    int pitch){
        
    unsigned char* fbs = (unsigned char*) fbv;
    
    char line[32];
    if( readline(fd,line) != SUCCESS ){
        kprintf("Readline error 1\n");
        return -1;
    }
    if( readline(fd,line) != SUCCESS ){
        kprintf("Readline error 2\n");
        return -1;
    }
    
    //parse width and height
    char* p = line;
    while(*p && *p != ' ')
        p++;
    *p=0;
    int w = atoi(line);
    p++;
    char* q=p;
    while(*q)
        q++;
    int h = atoi(p);
    if( readline(fd,line) != SUCCESS )
        return -1;
    //at the data
    int i,j;
    
    #define NR_ROWS 4
    static unsigned char one_row[640*3*NR_ROWS];

    for(i=0;i<h;i+=NR_ROWS){
        int m=0;
        if( 0 != readFully(fd,one_row,sizeof(one_row)) )
            return -1;
        int n=0;
        for(m=0;m<NR_ROWS;++m){
            unsigned char* fb = fbs + pitch*(i+m);
            for(j=0;j<w;j++){
                unsigned r = one_row[n++];
                unsigned g = one_row[n++];
                unsigned b = one_row[n++];
                r >>= (8-redmask);
                g >>= (8-greenmask);
                b >>= (8-bluemask);
                unsigned v = (b<<blueshift) | (g<<greenshift) | (r<<redshift);
                if( bpp == 16 ){
                    *fb = v&0xff;
                    fb++;
                    *fb = (v>>8) & 0xff;
                    fb++;
                } else {
                    *fb = v&0xff;
                    fb++;
                    *fb = (v>>8) & 0xff;
                    fb++;
                    *fb = (v>>16) & 0xff;
                    fb++;
                }
                
            }
        }
    }
    return 0;
}

void sweet(int sweetness, void* framebuffer_start,
    int redshift, int redmask, int greenshift, int greenmask,
    int blueshift, int bluemask, int bpp, int pitch ){
    unsigned char* fb = (unsigned char*)framebuffer_start;
    if( sweetness == 0 ){
        int fd = file_open("flower1.ppm",0);
        if(fd<0){
            kprintf("Cannot open file");
            return;
        }    
        if( 0 != displayPPM(fd,fb,redshift,redmask,
                greenshift,greenmask, blueshift,bluemask,bpp,
                pitch) ){
            return;
        }
    }
    else{
        int i;
        int rv;
        int fd = file_open("ripple.ppm",0);
        for(i=0; ; i=(i+1)%16){
            if(i == 0 ){
                rv = file_seek(fd,0,SEEK_SET);
                if(rv!=0){
                    kprintf("Seek error\n");
                    return;
                }
            }
            if( 0 != displayPPM(fd,fb,redshift,redmask,
                greenshift,greenmask, blueshift,bluemask,bpp,
                pitch)){
                return;
            }
        }
    }
}
