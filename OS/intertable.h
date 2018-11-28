/*
Code written by Thomas Gilman for Operating Systems One taught James Hudson.
Some functions and implementations of code use code from the OS slides,
the rest was written by Thomas Gilman.
*/
#pragma once

extern volatile unsigned jiffies;
extern unsigned Frequency;

#pragma pack(push,1)
struct InterruptFrame{
    unsigned eip;
    unsigned cs;
    unsigned eflags;
    unsigned esp;   //only used when undergoing
    unsigned ss;    //a ring transition
};
#pragma pack(pop)

__attribute__((interrupt))
void divideByZeroInterrupt(struct InterruptFrame* fr);                   //interrupt 0

__attribute__((interrupt))
void debugTrapInterrupt(struct InterruptFrame* fr);                      //interrupt 1

__attribute__((interrupt))
void NMIInterrupt(struct InterruptFrame* fr);                            //interrupt 2

__attribute__((interrupt))
void int3Interrupt(struct InterruptFrame* fr);                           //interrupt 3

__attribute__((interrupt))
void OverflowInterrupt(struct InterruptFrame* fr);                       //interrupt 4

__attribute__((interrupt))
void BCInterrupt(struct InterruptFrame* fr);                             //interrupt 5

__attribute__((interrupt))
void badOpcodeInterrupt(struct InterruptFrame* fr);                      //interrupt 6

__attribute__((interrupt))
void NFPUInterrupt(struct InterruptFrame* fr);                           //interrupt 7

__attribute__((interrupt))
void DFInterrupt(struct InterruptFrame* fr, unsigned code);              //interrupt 8

__attribute__((interrupt))
void FPUOInterrupt(struct InterruptFrame* fr);                           //interrupt 9

__attribute__((interrupt))
void BTSSInterrupt(struct InterruptFrame* fr, unsigned code);            //interrupt 10

__attribute__((interrupt))
void NoSegInterrupt(struct InterruptFrame* fr, unsigned code);           //interrupt 11

__attribute__((interrupt))
void SFaultInterrupt(struct InterruptFrame* fr, unsigned code);         //interrupt 12

__attribute__((interrupt))
void GFaultInterrupt(struct InterruptFrame* fr, unsigned code);         //interrupt 13

__attribute__((interrupt))
void PFaultInterrupt(struct InterruptFrame* fr, unsigned code);         //interrupt 14

__attribute__((interrupt))
void MFaultInterrupt(struct InterruptFrame* fr);                         //interrupt 16

__attribute__((interrupt))
void MisalignedInterrupt(struct InterruptFrame* fr, unsigned code);      //interrupt 17

__attribute__((interrupt))
void MCheckInterrupt(struct InterruptFrame* fr);                         //interrupt 18

__attribute__((interrupt))
void SIMDInterrupt(struct InterruptFrame* fr);                           //interrupt 19

__attribute__((interrupt))
void VMInterrupt(struct InterruptFrame* fr);                             //interrupt 20

__attribute__((interrupt))
void unknownInterrupt(struct InterruptFrame* fr);

__attribute__((interrupt))
void unknownInterruptWithCode(struct InterruptFrame* fr, unsigned code);

__attribute__((interrupt))
void timerInterrupt(struct InterruptFrame* fr);      //interrupt 32

__attribute__((interrupt))
void KeyboardInterrupt(struct InterruptFrame* fr);   //interrupt 33

__attribute__((interrupt))
void CascadeInterrupt(struct InterruptFrame* fr);    //interrupt 34

__attribute__((interrupt))
void S2Interrupt(struct InterruptFrame* fr);         //interrupt 35

__attribute__((interrupt))
void S1Interrupt(struct InterruptFrame* fr);         //interrupt 36

__attribute__((interrupt))
void Av0Interrupt(struct InterruptFrame* fr);        //interrupt 37

__attribute__((interrupt))
void FloppyInterrupt(struct InterruptFrame* fr);     //interrupt 38

__attribute__((interrupt))
void ParPortInterrupt(struct InterruptFrame* fr);    //interrupt 39

__attribute__((interrupt))
void int40trap(struct InterruptFrame* fr);           //interrupt 40 RTC

__attribute__((interrupt))
void VidInterrupt(struct InterruptFrame* fr);        //interrupt 41

__attribute__((interrupt))
void Av1Interrupt(struct InterruptFrame* fr);        //interrupt 42

__attribute__((interrupt))
void Av2Interrupt(struct InterruptFrame* fr);        //interrupt 43

__attribute__((interrupt))
void MouseInterrupt(struct InterruptFrame* fr);      //interrupt 44

__attribute__((interrupt))
void FPUInterrupt(struct InterruptFrame* fr);        //interrupt 45

__attribute__((interrupt))
void DskC0Interrupt(struct InterruptFrame* fr);      //interrupt 46

__attribute__((interrupt))
void DskC1Interrupt(struct InterruptFrame* fr);      //interrupt 47

__attribute__((__interrupt__))
void syscallInterrupt(struct InterruptFrame* fr);