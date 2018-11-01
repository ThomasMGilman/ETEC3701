int main(int argc, char* argv[])
{
    asm("hlt");
    register unsigned flag asm("esi");
    flag=0x31337;
    while(flag){
    }
    return 0;
}