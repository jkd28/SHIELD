#include <avr/interrupt.h>
#include <Time.h>

_Bool trigger;
_Bool transmit;


void wait(volatile int time_multiple, volatile char time_choice);
void delay_T_msec_timer1(volatile char time_choice);

//kHz Sample 
ISR(TIMER0_COMPA_vect)
{
    TCNT0 = 0; //MUST RESET TIMER IMMEDIATELY AFTER INTERRUPT FOR ACCURATE TIMING
    trigger = 1;
}

//Data Enable Interrupt; PIN D2
ISR(INT0_vect)
{
  wait(2,0);  //debounce
  transmit = (transmit xor ((PIND & 0x04)>>2));
  
  if (transmit){
    startTimer();
  } else {
    stopTimer();
  }
  
  EIFR = EIFR | 1 << INTF0;     //Clear INT0 Flag
}

void TIMER_INIT(){
  TCCR0A = 0x00;
  OCR0A = 0xF9; //compare level for 1 millisecond 
}

void startTimer(){
  TCNT0 = 0x7C; //pre-load timer to 124, 
  TCCR0B = 1 <<CS21 | 1<<CS20; //TCCR0B = 0x03, pre-scaler 64;
  TIMSK0 = (1 << OCIE0A);
}

void stopTimer(){
  TIMSK0 = (0 << OCIE0A);
}

int main(void)
{
  DDRC = 0xFF;  //data on C0
  DDRB=0x0F;    //trigger snoop on B0, data input on B4
  transmit = 0;
  _Bool new_bit;

  //###############   PIN INTERRRUPT SETUP  #####################
  DDRD = DDRD | (0<<PORTD2)| (0<<PORTD3);  //D2 Interrupt set to input
  EICRA = 1<<ISC01 | 1<<ISC00; //set INT0 to trigger on logic change
  EIMSK = 1<<INT0; // enable INT0
  sei();
  //#################################################################

  TIMER_INIT();
  
  while(1){
    
    PORTB = trigger;
    if(trigger){
      trigger = 0;

      new_bit = (PIND & 0x08)>>3;
      
      PORTC = new_bit; 
    }  
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
