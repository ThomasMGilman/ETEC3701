#include "console.h"

#define FrameWidth mbi->mbiFramebufferWidth
#define FrameHeight mbi->mbiFramebufferHeight
#define FramePitch mbi->mbiFramebufferPitch

struct MultibootInfo *mbi;				//my MultiBootInfo
volatile unsigned char* framebuffer;	//FrameBuffer
unsigned int pixCol = 0;				//Screen - X cord
unsigned int pixRow = 0;				//Screen - Y cord
char lastCharDrawn;						//save last char in case it is made bold

char debugMsg[50];

//stuff for transitions of colors
unsigned int colorChangeIndex = 0;
unsigned int colorPattern[6][3] = {{255,0,0}, {255,165,0}, {255,255,0}, {0,255,0}, {0,0,255}, {128,0,128}}; //Red,Orange,Yellow,Green,Blue,Purple
unsigned int red = 255, green = 255, blue = 255; 															//Color Variables (set to White)

int ksprintf(char* s, const char* fmt, ... ) __attribute__((format (printf , 2, 3 ) )); //kprintf func

void loop(void)
{
	for(volatile int i = 0; i < 1000000000; i++);
}
void set_pixel(int x, int y, int r, int g, int b)
{
	r >>= (8 - mbi->mbiFramebufferRedMask);
	g >>= (8 - mbi->mbiFramebufferGreenMask);
	b >>= (8 - mbi->mbiFramebufferBlueMask);

	unsigned short colorValue = (b << mbi->mbiFramebufferBluePos) | (g << mbi->mbiFramebufferGreenPos) | (r << mbi->mbiFramebufferRedPos);
	((unsigned short*)framebuffer)[(x+y*(FramePitch/2))] = colorValue;
}
void clearScreen(void)
{
	for(pixCol = 0; pixCol < FrameWidth; (pixCol)++)
	{
		for(pixRow = 0; pixRow < FrameHeight; (pixRow)++)
			set_pixel(pixCol, pixRow, 0, 0, 0);
	}
	pixCol = 0; pixRow = 0;
}
void smoothlyTransitionColors(void)
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
void backspace(void)
{
	if((pixCol == 0 || pixCol < CHAR_WIDTH) && pixRow > 0)	//backspacing at X index 0
	{
		pixRow -= CHAR_HEIGHT;
		pixCol = FrameWidth - CHAR_WIDTH;
	}
	else if(pixCol > 0 && pixCol >= CHAR_WIDTH)
		pixCol -= CHAR_WIDTH;
	else
		pixCol = 0;
}
void newLine(void)
{
	if(pixRow + CHAR_HEIGHT > FrameHeight - CHAR_HEIGHT)
	{
		scroll();
	}
	else
	{
		pixCol = 0;
		pixRow += CHAR_HEIGHT;
	}
}
void scroll(void)
{
	kmemcpy((void*)(framebuffer), (const void*)(framebuffer+(CHAR_HEIGHT*FramePitch)), 
		(FrameHeight - CHAR_HEIGHT)*(FramePitch));
	kmemset((void*)(framebuffer+((FrameHeight-CHAR_HEIGHT)*FramePitch)), CHAR_HEIGHT*FramePitch);
	pixCol = 0;
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
			--pixCol;
			consoleDrawChar(lastCharDrawn,1);
			++pixCol;
			break;
		case(127)://delete
			backspace();
			consoleDrawChar((char)32,0);
			backspace();
			break;
		case(9)://\t move to next pos divisable by 8
			tabVal = ((FrameWidth - pixCol)/CHAR_WIDTH) % 8;
			if(tabVal == 0)
				pixCol += CHAR_WIDTH*8;
			else
				pixCol += (tabVal * CHAR_WIDTH);
			if(pixCol > FrameWidth - CHAR_WIDTH)
				newLine();
			break;
		case(10)://\n newline
			newLine();
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
	if(pixCol > FrameWidth - CHAR_WIDTH)
		newLine();
	lastCharDrawn = c;
}
void consoleDrawString(char* myString)
{
	int indexOfString = 0;
	while(myString[indexOfString])									//while not NULL
		console_putc(myString[indexOfString++]);					//print the char to screen from bottom left
}
int consol_init(struct MultibootInfo *m)
{
	mbi = m;
    framebuffer = (volatile unsigned char*) (unsigned)mbi->mbiFramebufferAddress;
	clearScreen();
	return 0;
}