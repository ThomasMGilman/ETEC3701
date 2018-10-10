#include "util.h"
#include "font.h"
#pragma once

void set_pixel(int x, int y, int r, int g, int b);

void smoothlyTransitionColors();

void clearScreen();

void backspace();

void newLine();

void scroll();

void consoleDrawChar(char ch, int bold);

void loop();

void console_putc(char c);

void consoleDrawString(char* myString);

void sweet();

void consol_init(struct MultibootInfo *m);