#include "console.h"

struct MultibootInfo *mbi;				//my MultiBootInfo
volatile unsigned char* framebuffer;	//address of FrameBuffer
unsigned int pixCol = 0;				//Screen - X cord
unsigned int pixRow = 0;				//Screen - Y cord
char lastCharDrawn;						//save last char in case it is made bold

//stuff for transitions of colors
unsigned int colorChangeIndex = 0;
unsigned int colorPattern[6][3] = {{255,0,0}, {255,165,0}, {255,255,0}, {0,255,0}, {0,0,255}, {128,0,128}}; //Red,Orange,Yellow,Green,Blue,Purple
unsigned int red = 255, green = 255, blue = 0; 																//Color Variables (set to Yellow)

void set_pixel(int x, int y, int r, int g, int b)
{
	r >>= (8 - mbi->mbiFramebufferRedMask);
	g >>= (8 - mbi->mbiFramebufferGreenMask);
	b >>= (8 - mbi->mbiFramebufferBlueMask);

	unsigned short colorValue = (b << mbi->mbiFramebufferBluePos) | (g << mbi->mbiFramebufferGreenPos) | (r << mbi->mbiFramebufferRedPos);
	((unsigned short*)framebuffer)[(x+y*mbi->mbiFramebufferPitch)>>1] = colorValue;
}
void clearScreen()
{
	for(pixCol = 0; pixCol < mbi->mbiFramebufferWidth*2; (pixCol)++)
	{
		for(pixRow = 0; pixRow < mbi->mbiFramebufferHeight; (pixRow)++)
			set_pixel(pixCol, pixRow, 0, 0, 0);
	}
	pixCol = 0; pixRow = 0;
}
void smoothlyTransitionColors()
{
	if(red != colorPattern[colorChangeIndex][0] || green != colorPattern[colorChangeIndex][1] || blue != colorPattern[colorChangeIndex][2])
	{
		if(red < colorPattern[colorChangeIndex][0]) red++;
		else if(red > colorPattern[colorChangeIndex][0]) red--;
		if(green < colorPattern[colorChangeIndex][1]) green++;
		else if( green > colorPattern[colorChangeIndex][1]) green--;
		if(blue < colorPattern[colorChangeIndex][2]) blue++;
		else if (blue > colorPattern[colorChangeIndex][2]) blue--;
	}
	else
	{
		if(colorChangeIndex == 5) colorChangeIndex = 0;
		else colorChangeIndex++;
	}
}
void backspace()
{
	if(pixCol == 0 && pixRow > 0)	//backspacing at X index 0
	{
		(pixRow) -= CHAR_HEIGHT;
		pixCol = mbi->mbiFramebufferWidth*2 - CHAR_WIDTH;
	}
	else
		pixCol -= CHAR_WIDTH;
}
void newline()
{
	if(pixRow < mbi->mbiFramebufferHeight)
	{
		pixCol = 0;
		pixRow += CHAR_HEIGHT;
	}
}
void consoleDrawChar(char ch, int bold)
{
	const int *c = font_data[(unsigned int)ch];
	int cx,cy;
	for(cy = 0; cy < CHAR_HEIGHT; cy++)
	{
		for(cx = 0; cx < CHAR_WIDTH; cx++)
		{
			if((MASK_VALUE >> cx) & c[cy]) 								//toggle the bit at index cx of 16bits
				set_pixel(pixCol+cx, pixRow+cy, red, green, blue);
			else if(!bold)
				set_pixel(pixCol+cx, pixRow+cy, 0, 0, 0);				//erase anything that isnt part of the char
		}
	}
	pixCol	+=	CHAR_WIDTH;												//move pixel pos to the right by CHAR_WIDTH
}
void console_putc(char c)
{
	unsigned int asciiVal = (unsigned int)c;
	unsigned tabVal = 0;
	switch(asciiVal)
	{
		case(8)://\b bold
			backspace();
			++pixCol;
			consoleDrawChar(lastCharDrawn,1);
			--pixCol;
			break;
		case(127)://delete
			backspace();
			if(asciiVal == 127)//delete portion
			{
				consoleDrawChar((char)32,0);
				backspace();
			}
			break;
		case(9)://\t tab move 8 chars over or to next line if run off screen
			tabVal = ((mbi->mbiFramebufferWidth - pixCol)/CHAR_WIDTH) % 8;
			if(tabVal == 0)
				pixCol += CHAR_WIDTH*8;
			else
				pixCol += (tabVal * CHAR_WIDTH);
			if(pixCol > mbi->mbiFramebufferWidth - CHAR_WIDTH)
				newline();
			break;
		case(10)://\n newline
			newline();
			break;
		case(12)://\f formfeed
			clearScreen();
			break;
		case(13)://\r return
			pixCol = 0;
			break;
		default:
			consoleDrawChar(c,0);
			break;
	}
	lastCharDrawn = c;
}
void consoleDrawString(char* myString)
{
	int indexOfString = 0;
	while(myString[indexOfString])									//while not NULL
		console_putc(myString[indexOfString++]);					//print the char to screen from bottom left
}
void consol_init(struct MultibootInfo *m)
{
	mbi = m;
    framebuffer = (volatile unsigned char*) (unsigned)mbi->mbiFramebufferAddress;
	clearScreen();
	//char* myName = "Thomas Gilman\nThomaGil\f\tThomasGilman\n\rC\nHI\nHoo\b\x7f";
	//consoleDrawString(myName);
}