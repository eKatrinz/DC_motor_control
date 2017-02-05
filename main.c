#define F_CPU 16000000UL

#define baudrate 9600L
#define bauddivider ((F_CPU/(16*baudrate))-1)

#define HIbit(x) ((x)>>8)
#define LObit(x) ((x)& 0xFF)

#define TIMER0_TACTS        0xFF
#define TIMER1_TACTS        0xFFFF
#define	TIMER0_PRESCALER	256
#define	TIMER_COUNT_1S	(((F_CPU/TIMER0_PRESCALER)/TIMER0_TACTS)+1)

#include <avr/io.h>
#include <avr/wdt.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include "LCD.h"

//////////////////////////////////////////////////////////////

static volatile u08 ms_ticks_byte;


static volatile unsigned int Hall_sensor_tick=0;
static volatile unsigned int *Encoder_tick;
static volatile unsigned int delta_tick;
static volatile unsigned int rps=0;
static volatile unsigned int current=0;
static volatile unsigned char FLAG;
static volatile unsigned char MODE=0;
static volatile unsigned char IF_UPDATE;

//////////////////////////////////////////////////////////////


//
void Global_Init(void)
{
cli();

DDRD &= ~(1<<PD2|1<<PD3|1<<PD4);
PORTD &= ~(1<<PD2|1<<PD3|1<<PD4);

ADMUX=0;
ADMUX|= 0<<REFS1|0<<REFS0;                  // ИОН на входе AREF
ADCSRA|=0<<ADEN|0<<ADIE|1<<ADPS2|1<<ADPS1|1<<ADPS0;  // Однократное преобразование. Прерывание АЦП запрещено. Частота выборок делится на 128
ADCSRB|=0<<BIN|1<<ADLAR|0<<ACME|0<<ADTS2|0<<ADTS1|0<<ADTS0;
DIDR0=0b00000000;

EICRA = 1<<ISC11|0<<ISC10|1<<ISC01|0<<ISC00;
EIMSK = 1<<INT1|1<<INT0;

IF_UPDATE = 1;
Encoder_tick = &rps;

wdt_reset();
wdt_enable(WDTO_2S);   
   
sei();   
}

//
unsigned int ACP(unsigned char n) 
{ 
  unsigned int adcTemp = 0;
  
	ADMUX = (0b00000000)|n;
	ADCSRA |= 1<<ADEN;
    _delay_ms(30);
	
	cli();
   
    ADCSRA |= 1<<ADSC;                  // Запускаем преобразование
	while(!ADCSRA & 0b00010000) {};    // Ожидаем окончания преобразования
	
	ADCSRA |= 0<<ADIF;
    adcTemp = ADCL;
    adcTemp = ADCH<<2;
	ADCSRA |= 0<<ADEN;          // Выключаем АЦП + останавливаем преобразование АЦП = ADCSRA & 0b00111111; 
	
	sei();
	
	return (ADCW>>6);
}

//
ISR(INT0_vect)
{
	
	cli();
	
	if(MODE = ~MODE)
		Encoder_tick = &current;
	else 
		Encoder_tick = &rps;
	
	IF_UPDATE = 1;
	
	wdt_reset();
	
	sei();
}

//
ISR(INT1_vect)
{
    cli();
	
	char p = PIND;
	
	if((p>>PD4)&1)
	
	{	(*Encoder_tick)--;  }
		
	else
	
	{	(*Encoder_tick)++; }
		
	wdt_reset();
	
	IF_UPDATE = 1;
	
	sei();
}
void writeNumber(unsigned int value){
	nokia_2putchar(value/10000+48);
	nokia_2putchar(value%10000/1000+48); 
	nokia_2putchar(value%10000%1000/100+48);
	nokia_2putchar(value%10000%1000%100/10+48);
	nokia_2putchar(value%10000%1000%100%10+48);
}
void writeRPM(void){
	nokia_gotoxy(11, 0);
	nokia_puts_prgm(PSTR("обр"));

	nokia_gotoxy(11, 1);
	nokia_puts_prgm(PSTR("мин"));
}

void writeCurrent(void){
	nokia_gotoxy(11, 2);
	nokia_2putchar('A');
}

void writeRPMValue(unsigned int value){
	nokia_gotoxy(0, 0);
	writeNumber(value);
}
void writeCurrentValue(unsigned int value){
	nokia_gotoxy(0, 2);
	writeNumber(value);
}
void writeMode(unsigned char mode){
	nokia_gotoxy(0, 4);
	if(mode)
		nokia_2putchar('O');
	else
	
		nokia_2putchar('T');
}


//////////////////////////////////////////////////////////////  
int main(void)
{
	

    Global_Init();
	
	nokia_init();

	

	while(1)
	{
		if(IF_UPDATE){
			IF_UPDATE = 0;
			nokia_clear();
			
			writeRPMValue(rps);
			writeRPM();
			writeCurrentValue(current);
			writeCurrent();
			writeMode(MODE);
		//	nokia_gotoxy(11, 2);
	//		nokia_puts_prgm(PSTR("киря"));
			
			
		}
		_delay_ms(1);
		wdt_reset();

	}

	return 0;
}