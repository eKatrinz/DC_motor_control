#define F_CPU 1000000UL

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

#define MODE_QUANTITY 3
#define CURRENT_MODE 0
#define RPS_MODE 1
#define POTENT_MODE 2



typedef struct {
	unsigned int encoder;
	
	char name;
} Mode;

//////////////////////////////////////////////////////////////

volatile u08 ms_ticks_byte;



volatile Mode mode_array[MODE_QUANTITY];
volatile unsigned char global_mode;

volatile unsigned char IF_UPDATE;

//////////////////////////////////////////////////////////////


//
void modeInit(){
	mode_array[CURRENT_MODE].encoder = 0;
	mode_array[CURRENT_MODE].name = 'T';
	
	mode_array[RPS_MODE].encoder = 0;
	mode_array[RPS_MODE].name = 'O';

	mode_array[POTENT_MODE].encoder = 0;
	mode_array[POTENT_MODE].name = 'П';
	
	global_mode=0;
}

void init_pwm (void)
{
	DDRD |= 1<<PD5;
	
	TCCR0A=(1<<COM0B1)|(0<<WGM02)|(0<<WGM01)|(1<<WGM00); //На выводе OC1A единица, когда OCR1A==TCNT1, восьмибитный ШИМ
	TCCR0B=(1<<CS00);//|(1<<CS02);		 //Делитель= /1024
	OCR0B=0x88;			//Начальная яркость
}


void Global_Init(void)
{
cli();

DDRD &= ~(1<<PD2|1<<PD3|1<<PD4);

PORTD |= (1<<PD2|1<<PD3|1<<PD4);

EICRA = 1<<ISC11|0<<ISC10|1<<ISC01|0<<ISC00;
EIMSK = 1<<INT1|1<<INT0;

IF_UPDATE = 1;
modeInit();

wdt_reset();
wdt_enable(WDTO_2S);   
   
sei();   
}


//
ISR(INT0_vect)
{
	
	cli();
	if(++global_mode>=MODE_QUANTITY) global_mode = 0;
	
	IF_UPDATE = 1;
	
	wdt_reset();
	
	sei();
}

//
ISR(INT1_vect)
{
//    cli();

	char p = PIND;
//	_delay_us(100);
	if(!((PIND>>PD3)&1)){
		if((p>>PD4)&1){
			if(mode_array[global_mode].encoder)
				mode_array[global_mode].encoder--; 
		}else{
			mode_array[global_mode].encoder++; 
		}
		IF_UPDATE = 1;
		EIFR &= ~(1<<INTF1);
	}	
//	_delay_ms(100);
	wdt_reset();

//	sei();
}
void writeNumber(unsigned int value){
	unsigned int _val;
	char _flag = 0;
	if(_flag | (_val = value/10000)) { nokia_2putchar(_val+48); _flag=1;}
	if(_flag | (_val = value%10000/1000)) { nokia_2putchar(_val+48); _flag=1;}
	if(_flag | (_val = value%10000%1000/100)) { nokia_2putchar(_val+48); _flag=1;}
	if(_flag | (_val = value%10000%1000%100/10)) { nokia_2putchar(_val+48); _flag=1;}
	_val = value%10000%1000%100%10;
	nokia_2putchar(_val+48);
}
void writeRPM(){
	nokia_gotoxy(11, 0);
	nokia_puts_prgm(PSTR("обр"));

	nokia_gotoxy(11, 1);
	nokia_puts_prgm(PSTR("мин"));
}

void writeCurrent(){
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
void writeMode(){
	nokia_gotoxy(0, 4);
	nokia_2putchar(mode_array[global_mode].name);
	nokia_gotoxy(11, 4);
	writeNumber(global_mode);
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
			
			writeRPMValue(mode_array[RPS_MODE].encoder);
			writeRPM();
			writeCurrentValue(mode_array[CURRENT_MODE].encoder);
			writeCurrent();
			writeMode();
		//	nokia_gotoxy(11, 2);
	//		nokia_puts_prgm(PSTR("киря"));
			
			
		}
		_delay_ms(10);
		wdt_reset();

	}

	return 0;
}