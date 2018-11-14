

#ifdef DRY_RUN
//stub for standalone tests

#define SYSCALL_PLAY 1
#define SYSCALL_SLEEP 2
#define SYSCALL_HALT 3
#define SYSCALL_WRITE 4

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int do_syscall(int p0, int p1, int p2, int p3){
    switch(p0){
        case SYSCALL_PLAY:
            if(  p1 != 0 ){
                printf("\t\tPlay: %d\n", (int) p1);
            }
            break;
        case SYSCALL_HALT:
            exit(0);
            break;
        case SYSCALL_SLEEP:
            usleep(p1*1000);
            break;
        case SYSCALL_WRITE:
            printf("%.*s|", (int)p3, (char*)p2 );
            break;
        default:
            printf("?%d?\n",(int)p0);
            ;
    }
    return 0;
}
        
#else
#include "syscalls.h"
int do_syscall(int p0, int p1, int p2, int p3){
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
#endif

//https://imslp.org/wiki/100_Songs_of_England_(Bantock%2C_Granville)
//http://pages.mtu.edu/~suits/notefreqs.html
const char notes[] = 
  {"e'8 a'4 a'8 a'8 g'8 f'8 e'4 d'8 c'4 e'8 a'4 a'8 b'4 g'8 a'1 e'8 "
  "a'4 b'8 c''4 d''8 e''4 c''8 a'4 b'8 c''4 c''8 c''8 b'8 a'8 b'1 e'8 "
  "a'4 b'8 c''4 d''8 e''4 c''8 a'4 b'8 c''4 c''8 c''8 b'8 a'8 b'2 c''4 b'8 "
  "a'4 a'8 a'8 g'8 f'8 e'4 d'8 c'4 e'16 e'16 a'4 a'8 b'4 g'8 a'2 h4 "
  
  "e'8 a'4 a'8 a'8 g'8 f'8 e'4 d'8 c'4 e'8 a'4 a'8 b'4 g'8 a'1 e'8 "
  "a'4 b'8 c''4 d''8 e''4 c''8 a'4 b'8 c''4 c''8 c''8 b'8 a'8 b'1 e'8 "
  "a'4 b'8 c''4 d''8 e''4 c''8 a'4 b'16 b'16 c''4 c''8 c''8 b'8 a'8 b'2 c''4 b'8 "
  "a'4 a'8 a'8 g'8 f'8 e'4 d'8 c'4 e'16 e'16 a'4 a'8 b'4 g'8 a'1"};  

const char lyrics[] =
    {"As me _ and _ my com*_*rade were set*ting four or five \nAnd "
    "tak*ing on 'em up a*gain, we caught the hare _ a*live: \nWe "
    "took the hare a*live, my boys, and thro' the woods _ did steer. \nOh! 'tis "
    "my de*light on a shin*ing night in the sea*son of the year. _ \n\n"
    
    "I threw it on _ my shoul*_*der, and then we trudg*ed home; \nWe "
    "took it to a neigh*bor's house and sold it for _ a crown; \nWe "
    "sold it for a crown, my boys, but I did not tell _ you where. \nOh! 'tis "
    "my de*light on a shin*ing night in the sea*son of the year.\n"};
     
//C major    
//    440,494,262,294,330,349,392,     //a4,b4,...g4
//A major: C#, F#, G#
//    440,494,277,294,330,370,415,     //a4,b4,...g4

const unsigned HZ[] = {
    440,494,277,294,330,370,415,0
};

union timeU{
    unsigned char c[4];
    unsigned v;
};

int mystrlen(const char* txt){
    int num=0;
    while(*txt++)
        num++;
    return num;
}

//msec per quarter note
#define QUARTERNOTE 300

const char userMsg = 'H';

int main(int argc, char* argv[])
{
    unsigned i=0;
    unsigned li=0;
    while(notes[i]){
        if(!notes[i])
            i=0;
            
        while( notes[i] && notes[i] == ' ')
            i++;
        char key = notes[i++];
        int octave=-1;
        while(notes[i]=='\''){
            octave++;
            i++;
        }
        while(notes[i]==','){
            octave--;
            i++;
        }
        unsigned hz = HZ[key-'a'];
        while(octave>0){
            octave--;
            hz*=2;
        }
        while(octave<0){
            octave++;
            hz/=2;
        }
        union timeU time;
        time.c[0]=time.c[1]=time.c[2]=time.c[3]=0;
        int j=0;
        while( notes[i] && notes[i] != ' '){
            time.c[j++] = notes[i++];
        }
        
        unsigned durationMS;
        switch(time.v){
            case '1':
                durationMS = QUARTERNOTE*4;
                break;
            case '4':
                durationMS = QUARTERNOTE;
                break;
            case '8':
                durationMS = QUARTERNOTE/2;
                break;
            case 0x3631:        //'16'
                durationMS = QUARTERNOTE/4;
                break;
            case '2':
                durationMS = QUARTERNOTE*2;
                break;
            case 0x2e34:        //'4.'
                durationMS = QUARTERNOTE + QUARTERNOTE/2;
                break;
            default:
                durationMS = QUARTERNOTE * 10;
        }
        
        unsigned k=li;
        //advance k to either end of string or * or space
        while(lyrics[k] && lyrics[k] != '*' && lyrics[k] != ' ' )
            k++;
        
        if( k != li ){
            unsigned end = k;
            if( lyrics[k] == ' ' || lyrics[k] == '\n')
                end++;  //also print the space
                
            //only print if it's not an underline
            if( lyrics[li] != '_' )
                do_syscall(SYSCALL_WRITE,1,(unsigned)(&lyrics[li]),end-li);
                
            if( lyrics[k] )
                li = k+1;
            else
                li = k;
        }
        
        do_syscall(SYSCALL_PLAY,hz,0,0);
        do_syscall(SYSCALL_SLEEP,durationMS,0,0);
        do_syscall(SYSCALL_PLAY,0,0,0);
        
    }
    //do_syscall(SYSCALL_LOG, 0, 0, 0); //made it here
    do_syscall(SYSCALL_PLAY,0,0,0);
    //do_syscall(SYSCALL_LOG, 0, 0, 0); //made it here
    while(1){
        do_syscall(SYSCALL_HALT,0,0,0);
    }
    return 0;
    
}

