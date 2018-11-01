/*
   ISR BASED FREQ SHIFT 

   Created: 3/1/2018 12:00:33 PM
   Author : lepra
*/

#include <avr/io.h>
#include <avr/interrupt.h>

_Bool F_Sig = 0;
_Bool trigger = 0;
ISR(TIMER0_COMPA_vect)
{
  TCNT0 = 0;
  trigger = 1;
}

int main(void)
{
  //OCR2A sets top level of counter/timer 2
  //Run PWM duty cycle off of OCR2B

  //#############   DATA INIT    ###########################  
  char c = 'A'; //0100 0001
  uint8_t data_B = c;
  _Bool data_b;
  int i = 0;
  //#############   PWM PRESET    ###########################
  DDRD = 0xFF; //analog pin out 3 is pwm output for pwm 2B
  OCR2A = 0x34; //carrier freq 2
  OCR2A = 0x40; //carrier freq 1
  OCR2B = 0x00; //set duty to 0%
  DDRC = 0x01;
  
  //TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20); //com2b1 enables pwm on analog 3, wgm bits set pwm type
  TCCR2B = 1 << WGM22 | 0 << CS22 | 1 << CS21 | 0 << CS20; //CS bits for pre-scale of 8
 
  //################    TIMER SETUP   ###########################
  TCCR0A = 0x00;
  TCNT0 = 0x00; //pre-load timer to 0
  OCR0A = 0xF9;
  TIMSK0 = (1 << OCIE0A);
  sei();
  TCCR0B = 1 <<CS21 | 1<<CS20; //TCCR0B = 0x03, pre-scaler 64;
  //#############################################################

  OCR2B = 0x1F; //start pwm
  
  while (1){

    //trigger set by clock interrupt every 1ms
    //data_B --> uint8_t working Byte from stack
    //data_b --> _Bool working bit from Byte
    //OCR2A --> top lvl of counter, controls PWM Freq
    PORTC = trigger;
    
    if(trigger){
      trigger = 0;

      if(i<8){
        data_b = (0b00000001 & (data_B >> i));
        i++;        
      }
      else{
        i=0;
        //retrieve new Byte data_B from stack
        data_b = (0b00000001 & data_B);
      }
      
      if(data_b){
        OCR2A = 0x3F;
        PORTD = 0xF0;
      }
      else if(!data_b){
        OCR2A = 0x30;
        PORTD = 0x00;
      }
    }
  }
}
