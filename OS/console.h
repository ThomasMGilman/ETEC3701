#include "util.h"
#include "font.h"
#pragma once

void set_pixel(int x, int y, int r, int g, int b);

void smoothlyTransitionColors(void);

void clearScreen(void);

void backspace(void);

void newLine(void);

void scroll(void);

void consoleDrawChar(char ch, int bold);

void loop(void);

void console_putc(char c);

void consoleDrawString(char* myString);

void consol_init(struct MultibootInfo *m);