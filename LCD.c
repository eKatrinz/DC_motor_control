

#include "LCD.h"


static volatile u08 nokia_contrast;
static volatile u08 nokia_inverse;
static volatile u08 nokia_x;
static volatile u08 nokia_y;

//
u08 nokia_where_x(void)
{
	return nokia_x;
}

//
u08 nokia_where_y(void)
{
	return nokia_y;
}

//
u08 nokia_get_contrast(void)
{
	return nokia_contrast;
}

//
u08 nokia_get_inverse(void)
{
	return nokia_inverse;
}

//
void nokia_w(u08 ch)
{
	for (u08 i = 8; i; i--)
	{
		if (ch & 0x80)	
		sbit(LCD_PORT,DIN_PIN);
		else			
		cbit(LCD_PORT,DIN_PIN);
		cbit(LCD_PORT,SCK_PIN);

		ch <<= 1;

		sbit(LCD_PORT,SCK_PIN);
		wdt_reset();
	}
}

//
void nokia_cmd(u08 cmd)
{
	cbit(LCD_PORT,DC_PIN);
	nokia_w(cmd);
}

//
void nokia_data(u08 ch)
{
	sbit(LCD_PORT,DC_PIN);

	nokia_w(ch);
	if (++nokia_x > GLCD_PIXELX-1)
	{
		nokia_x = 0;
		nokia_y++;
		wdt_reset();
	}
}

//
static void nokia_set_addr(u08 x, u08 y)
{
	if (y > 5) { y = 5; }
	if (x > GLCD_PIXELX - 1) { x = GLCD_PIXELX - 1; }
	wdt_reset();
	nokia_x = x;
	nokia_y = y;
	nokia_cmd(GLCD_SETYADDR | y);
	nokia_cmd(GLCD_SETXADDR | x);
}

//
void nokia_gotoxy(u08 x, u08 y)
{
	nokia_set_addr(((x<<1) + x) << 1, y);
}

//
void nokia_putchar(u08 ch)
{
	u16 pos = ch;
	
	pos += (pos << 2);

	for (ch = 5; ch; ch--)
	{ 
	  nokia_data(pgm_read_byte(&FONT5x7[pos++])); 
	  wdt_reset();
	}

	nokia_data(0);
}

//
void nokia_puts(const u08 *s)
{
	while (*s)
		nokia_putchar(*s++);
}

//
void nokia_puts_prgm(const char *s)
{
	register u08 c;
	while((c = pgm_read_byte(s++)))
	 {
		nokia_putchar(c); 
		wdt_reset(); 
	 }
}

//
void put_nibble(u08 b)
{
	u08 w = 0;

	if (b & 1) w |= 0x03;
	if (b & 2) w |= 0x0C;
	if (b & 4) w |= 0x30;
	if (b & 8) w |= 0xC0;

	nokia_data(w);
	nokia_data(w);
}

//
void nokia_2putchar(u08 ch)
{
	u08 b;
	u16 pos = ch;
	pos += (pos << 2);

	for (ch = 5; ch; ch--)
	{
		b = pgm_read_byte(&FONT5x7[pos++]);
		put_nibble(b);
		wdt_reset();
	}
	nokia_data(0);
	nokia_data(0);

	pos-=5;
	nokia_set_addr(nokia_where_x()-12,nokia_where_y()+1);


	for (ch = 5; ch; ch--)
	{
		b = pgm_read_byte(&FONT5x7[pos++]);
		b >>= 4;
		put_nibble(b);
		wdt_reset();
	}
	nokia_data(0);
	nokia_data(0);

	nokia_set_addr(nokia_where_x(),nokia_where_y()-1);
}

//
void nokia_2puts(u08 *s)
{
	while (*s)
	{ 
	  nokia_2putchar(*s++);
	  wdt_reset();
	}  
}

//
void nokia_set_contrast(u08 contrast)
{
	nokia_contrast = contrast;
	nokia_cmd(GLCD_FUNCTIONSETEXT);
	nokia_cmd(GLCD_SET_VOP | contrast);
	nokia_cmd(GLCD_FUNCTIONSET);
}

//
void nokia_set_inverse(u08 inv)
{
	nokia_inverse = inv;
	if(inv)
		{ nokia_cmd(GLCD_DISPLAYINVERT); }
	else
		{ nokia_cmd(GLCD_DISPLAYNORMAL); }
}

//
void nokia_init(void)
{
    wdt_reset();
	LCD_PORT|=(1<<SCK_PIN|1<<DIN_PIN|1<<DC_PIN|1<<RESET_PIN);
	
	LCD_DDR|=(1<<SCK_PIN|1<<DIN_PIN|1<<DC_PIN|1<<RESET_PIN);


	cbit(LCD_PORT,RESET_PIN);
	asm volatile ("nop"); asm volatile ("nop");
	asm volatile ("nop"); asm volatile ("nop");
	sbit(LCD_PORT,RESET_PIN);

	nokia_cmd(GLCD_FUNCTIONSETEXT);
	nokia_cmd(GLCD_TEMPCOEF);
	nokia_cmd(GLCD_SET_BIAS | GLCD_BIAS_1_10);
	nokia_set_contrast(24);
	nokia_set_inverse(FALSE);
	wdt_reset();
}

//
void nokia_clear(void)
{
	u08 i;
	
	nokia_gotoxy(0, 0);
	for(i = 252; i; i--)
	{ 
	  nokia_data(0); 
	  nokia_data(0); 
	}
	nokia_gotoxy(0, 0);
}


//////////////////////////////////////////////////////////////
