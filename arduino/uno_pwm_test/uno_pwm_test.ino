/*
 * Lab6.c
 *
 * Created: 3/1/2018 12:00:33 PM
 * Author : lepra
 */ 

#include <avr/io.h>
#include <avr/interrupt.h>

void wait(volatile int time_multiple, volatile char time_choice);
void delay_T_msec_timer1(volatile char time_choice);

int main(void)
{
  //OCR2A sets top level of counter/timer 2
  //Run PWM duty cycle off of OCR2B
  
  //#############   PWM PRESET    ###########################
  DDRD = 0xFF; //analog pin out 3 is pwm output for pwm 2B
  OCR2A = 0x34; //carrier freq 2
  OCR2A = 0x40; //carrier freq 1
  OCR2B = 0x00; //set duty to 0%
  
  //TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);  
  TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20); //com2b1 enables pwm on analog 3, wgm bits set pwm type
  TCCR2B = 1<<WGM22 | 0<<CS22 | 1<<CS21 | 0<<CS20; //CS bits for pre-scale of 8
  //#############################################################
  //int TC1= TCCR2B;
  //int TC2 = 0B00000010;
  OCR2B = 0x1F; //start pwm
  while (1)
  {
    /*OCR2A = 0x30;
    wait(8,1);
    OCR2A = 0x60;
    wait(8,1);
    OCR2A = 0xB0;
    
    wait(8,1);
    OCR2A = 0xFF;
    wait(8,1);    
    */
    //timing for freq changes. wait(time duration, prescaler choice)
    wait(16,1);
    OCR2A=0x30;
    wait(16,1);
    OCR2A=0x3F;
        
  }
}

void wait(volatile int time_multiple, volatile char time_choice)
{  
  while (time_multiple > 0 )
  {
    delay_T_msec_timer1(time_choice);
    time_multiple--;
  }
}


void delay_T_msec_timer1(volatile char time_choice)
{
  TCCR1A = 0x00;
  TCNT1 = 0; //pre-load timer1 to 0
  switch(time_choice) //start timer with scaler setting
  {
    case 1:
    TCCR1B = 1<<CS11; //TCCR1B = 0x02, pre-scaler 8
    break;
    case 2:
    TCCR1B = 1<<CS11 | 1<<CS10; //TCCR1B = 0x03, pre-scaler 64
    break;
    case 3:
    TCCR1B = 1<<CS12; //TCCR1B = 0x04, pre-scaler 256
    break;
    case 4:
    TCCR1B = 1<<CS12 | 1<CS10; //TCCR1B = 0x05, pre-scaler 1024
    break;
    default:
    TCCR1B = 1<<CS10; //TCCR1B = 0x01, no pre-scale
    break;
  }

  while (TCNT1 < 0xfa); //exits when count = 250
  TCCR1B = 0x00; //stop timer 1
  TIFR1 = 0x1<<TOV1; //clear overflow bit
}
