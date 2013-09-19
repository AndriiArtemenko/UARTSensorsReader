/*****************************************************
This program was produced by the
CodeWizardAVR V2.05.0 Evaluation
Automatic Program Generator
© Copyright 1998-2010 Pavel Haiduc, HP InfoTech s.r.l.
http://www.hpinfotech.com

Project : WiFly Device
Version : 0.1
Date    : 06.08.2013
Author  : Andrii Artemenko
Company : 
Comments: 


Chip type               : ATtiny2313
AVR Core Clock frequency: 1,000000 MHz
Memory model            : Tiny
External RAM size       : 0
Data Stack size         : 32
*****************************************************/

#include <tiny2313.h>
#include <delay.h>
#include <string.h> 
#include <stdlib.h>    
#include "ds18x20_v2.h"

void USART_Send(char* str);
//------------------Constants----------------------
#define FRAMING_ERROR (1<<FE)  
#define PARITY_ERROR (1<<UPE)  
#define DATA_OVERRUN (1<<DOR)

#define F_CPU 1000000L
#define BAUD_RATE 2400L

#define ELECTRIC_DEVIDER 640;
#define WATER_DEVIDER 1;

//------------------VARs---------------------------
// Counters.
unsigned long waterCounter = 0;
unsigned long waterCounterTic = 0;
unsigned long electricCounter = 0;
unsigned long electricCounterTic = 0;

eeprom unsigned long stored_waterCounter;
eeprom unsigned long stored_electricCounter;

char command;
bit electric_flag = 0;
bit water_flag = 0;
bit power_flag = 0;
bit bounce_flag = 0;

//------------------Common-------------------------
// Led blink.
void blink7(int value){
	PORTB.7 = 1;
	delay_ms(value);
	PORTB.7 = 0;
}
void blink6(int value){
	PORTB.6 = 1;
	delay_ms(value);
	PORTB.6 = 0;
}

// Convert longint to char[]
void mLtoA(unsigned long val, char* buf){
    unsigned char size=0, buf_tmp[10], *p_buf=buf_tmp;
    do{
        *p_buf++ = val%10 + '0';
        val/=10;
        size++;
    }while(val);
    while(size--){
        *buf++ = *--p_buf;
    }
    *buf=0x00;
}

// Load counters values from EEPROM.
void load_countable(){
    waterCounter = stored_waterCounter;
    electricCounter = stored_electricCounter;
}

// Save counters values to EEPROM.
void save_countable(){
    stored_waterCounter = waterCounter;
    stored_electricCounter = electricCounter; 
}


// Return general temperature value
// HOW TO get real value:
// int integer_part = temperature >> 4;
// int fractal_part  = (temperature & 0x000F)*10/16;
int get_temperature(){   
    int temperature = -1; 
 
    if (w1_init() > 0) { 
        temperature = ds18b20_temperature(0); 
        if (temperature > 1000){               
            temperature=(-1)*4096-temperature;            
        }                          
   } 
   return temperature; 
}

//------------------External interrupts------------
// Interrupt from water counter.
interrupt [EXT_INT0] void ext_int0_isr(void)
{   
    unsigned long int devider=WATER_DEVIDER; 
    if (bounce_flag == 0){  
        bounce_flag = 1;
        TIMSK=0x01;  
        waterCounterTic++; 
        if (waterCounterTic >= devider) {
            waterCounterTic = 0;
            waterCounter++;           
            water_flag = 1;
        } 
    }   
}

// Interrupt from electric counter.
interrupt [EXT_INT1] void ext_int1_isr(void)
{
    unsigned long int devider=ELECTRIC_DEVIDER;
    
    electricCounterTic++;
    if (electricCounterTic >= devider) {
        electricCounterTic = 0;
        electricCounter++;          
        electric_flag = 1;
    }
}

//-----------------Comparator interrupt----------------
// Power lost interrupt.
interrupt [ANA_COMP] void ana_comp_isr(void)
{
    save_countable();
    power_flag = 1;            
}

//--------------Timer 0 output compare A----------------
interrupt [TIM0_COMPA] void timer0_compa_isr(void)
{
    // Anti contact bounce.
    bounce_flag = 0;
    TIMSK=0x00;      
}

//----------------------UART---------------------------
// Init UART
void init_UART(){
    long int baud = F_CPU/(16*BAUD_RATE)-1;
    UBRRH = (unsigned char)(baud>>8);
	UBRRL = (unsigned char)baud;

    UCSRB = (1<<RXEN)|(1<<TXEN)|(1<<RXCIE);
    UCSRC = (1<<USBS)|(3<<UCSZ0);
}

// Send byte to UART.
void USART_Transmit(unsigned char data)
{
	while (!(UCSRA & (1<<UDRE)) );	
	UDR = data;
}

// Send string to UART.
void USART_Send(char* str)
{
    while (*str)
        USART_Transmit(*str++); 
    USART_Transmit(0x0d);    
}

// Execute recived command.
void execute(char command){	
    char Buffer[16];  
	switch(command)
	{
		case 0x61: {
			mLtoA(waterCounter, Buffer);
			break;
		}
		case 0x62: {
			mLtoA(electricCounter, Buffer);
			break;
		}
		case 0x63: {  
            mLtoA(get_temperature(), Buffer);   
			break;
		}
		case 0x64: {
			waterCounter=0;
            strcpy(Buffer, "OK"); 
			break;
		}  
		case 0x65: {
			electricCounter=0;
            strcpy(Buffer, "OK"); 
			break;
		}        
		default:   
           strcpy(Buffer, "unknown");
	} 
    USART_Send(Buffer);  
}

// UART recive interrupt
interrupt [USART_RXC] void usart_rx_isr(void)
{
    char status,data;
    status=UCSRA;
    data=UDR;
    if ((status & (FRAMING_ERROR | PARITY_ERROR | DATA_OVERRUN))==0)
    {
        if (data == 0xD) {
            execute(command);
        } else{
            if (data != 0xA )
                command = data;
        }
    };
}

//
void init_Hardware() {
	// PB0 is AIN0 -- make it an input
	// PB1 is AIN1 -- make it an input
	// PB6 pin as output for eye LED
	// PB7 pin as output for eye LED  
    DDRB = 0b11000000;
	
    // Output voltage off
    PORTB = 0b00000000;

	//Set external interrupter settings
	//DDRD &= ~(PD2); // PD2 -> Input
	//DDRD &= ~(PD3); // PD3 -> Input 
    DDRD = 0b00000000;
    
	GIMSK |= (1<<INT0);   // Enable int0 interrupt
	GIMSK |= (1<<INT1);   // Enable int1 interrupt

	// INT0 will be interrupt by follow side of impulse
	// INT1 will be interrupt by follow side of impulse
	MCUCR = 0b00001010; 
     

	//Set comparator settings
	ACSR  |=  (1<<ACI);    // clear Analog Comparator interrupt
	ACSR  |=
	(0<<ACD)   |           // Comparator ON
	//(0<<ACBG)  |           // Disconnect 1.23V reference from AIN0 (use AIN0 and AIN1 pins)
	(1<<ACIE)  |           // Comparator Interrupt enabled
	(1<<ACIS1) |           // set interrupt bit on positive edge
	(1<<ACIS0);            // (ACIS1 and ACIS0 == 11), so comparator do interrupt by rising side of impulse
    
    // Timer/Counter 0 initialization
    TCCR0A=0x02; // Set CTC mode(interrupt on math)
    TCCR0B=0x05; // Set 1/1024 clock prescaler  (0,977 kHz)
    TCNT0=0x00;  // Default timer0 value=0
    OCR0A=0xFF;  // Math A register value=FF
    OCR0B=0x00;  // Math A register value=FF 
    TIMSK=0x00;  // Disable timer         
}


void main(void)
{   
// Crystal Oscillator division factor: 8
#pragma optsize-
CLKPR=0x80;
CLKPR=0x03;
#ifdef _OPTIMIZE_SIZE_
#pragma optsize+
#endif
   
    init_Hardware();	
    init_UART();
    #asm("sei")   
              
    load_countable();
    USART_Send("hello!");
    while (1){
        if (electric_flag == 1){
            blink6(100);
            electric_flag = 0;    
        }  
        if (water_flag == 1){
            blink6(100);
            water_flag = 0;    
        } 
        if (power_flag == 1){
            blink7(100);
            power_flag = 0;    
        } 
            
    }
}
