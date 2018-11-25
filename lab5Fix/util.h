#pragma once

#pragma pack(push,1)
    struct MultibootInfo
    {
        unsigned flags;
        unsigned memLower;  //always 640K
        unsigned memUpper;  //amount of memory above 1MB
        unsigned long long mbiFramebufferAddress;
        unsigned mbiFramebufferPitch;   //The distance from the start of first address of pixel and next pixel address
        unsigned mbiFramebufferWidth;
        unsigned mbiFramebufferHeight;
        unsigned char mbiFramebufferBpp;    //bits per pixel, (often 16 or 24bits (truecolor))
        unsigned char mbiFramebufferType;
        unsigned char mbiFramebufferRedPos;
        unsigned char mbiFramebufferRedMask;
        unsigned char mbiFramebufferGreenPos;
        unsigned char mbiFramebufferGreenMask;
        unsigned char mbiFramebufferBluePos;
        unsigned char mbiFramebufferBlueMask;
    };
#pragma pack(pop)

void clearBss(char* bssStart, char* bssEnd);

int Factorial(int num);