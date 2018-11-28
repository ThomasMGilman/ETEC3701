/*
Code written by Thomas Gilman for Operating Systems One taught James Hudson.
Some functions and implementations of code use code from the OS slides,
the rest was written by Thomas Gilman.
*/
#include "util.h"
#include "font.h"
#pragma once

void set_pixel(int x, int y, int r, int g, int b);

void smoothlyTransitionColors(void);

void clearScreen(void);

void backspace(void);

void boldOrBackspace(char c);

void newLine(void);

void scroll(void);

void consoleDrawChar(char ch, int bold);

void loop(void);

void console_putc(char c);

void consoleDrawString(char* myString);

int consol_init(struct MultibootInfo *m);