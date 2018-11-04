int main(int argc, char* argv[])
{
    register unsigned flag asm("esi");
    // while(1)
    // {;}
    flag=0x31337;
    while(flag)
    {;}
    return 0;
} 