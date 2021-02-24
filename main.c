#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

// ATTINY13A
//
//                         +-\/-+
// RESET PCINT5 ADC0 PB5  1|    |8  VCC
//  CLKI PCINT3 ADC3 PB3  2|    |7  PB2 ADC1 PCINT2 T0 SCK
//       PCINT4 ADC2 PB4  3|    |6  PB1 AIN1 PCINT1 OC0B INT0 MISO
//                   GND  4|    |5  PB0 AIN0 PCINT0 OC0A MOSI
//                         +----+
//

#include "serial.h"

void setup() {
    TCCR0B |= (1<<CS02) | (1<<CS00);  //prescale timer
    TIMSK0 |= (1<<TOIE0); //enable timer overflow interrupt

  	DDRB |= (1 << PB4); // PB4 button as output
    
  	DDRB &= ~(1 << PB1); // PB1 button as input

    // Set timer on PB1
    PCMSK |= (1<<PCINT1); // tell pin change mask to listen to PB1
    GIMSK |= (1<<PCIE); // enable PCINT interrupt in the general interrupt mask

	sei();
}

#define DIFF(x,y) ((x<y)?(y-x):(x-y))

#define I_SHORT 3 // 3 1*350 / 3 1*330
#define I_LONG 9 // 10 3*350 / 9 3*330 
#define I_SYNC 90 // 105 31*350 / 100 31*330

#define CLEAR_COMMAND { \
    length = 0;\
    command = 0; \
    sync_readed = 0; \
}

uint32_t command;
uint8_t length;
uint8_t sync_readed;

void handleCommand()
{
    TxByte('#');
    TxByte(4);
	SEND_32(command)
}

ISR(PCINT0_vect)
{
    cli();
    if (PINB & (1 << PB1)) // Rising
    {
        if (sync_readed == 1 && DIFF(TCNT0, I_SHORT) < 2) { // in sync mode read 1
            command <<= 1;
            command |= 1;
            length++;
        }
        else if (sync_readed == 1 && DIFF(TCNT0, I_LONG) < 2) { // in sync mode read 0
            command <<= 1;
            length++;
        }
        else if (sync_readed == 1 && DIFF(TCNT0, I_SYNC) < 14) { // leave sync mode
        	if (length >= 8) { // at least 8 bits needed
	            handleCommand();
        	}
	        CLEAR_COMMAND;
     	} else if (sync_readed == 0 && DIFF(TCNT0, I_SYNC) < 14) { // enter sync mode
	        CLEAR_COMMAND;
            sync_readed = 1;
        } else {
            CLEAR_COMMAND;
        }
    }
    
    TCNT0 = 0;
    
    sei();
}

ISR(TIM0_OVF_vect)
{  
    CLEAR_COMMAND;
}

void loop() {
}

int main(){
  setup();
  while(1){loop();}
  return 0;
}
