#include <avr/interrupt.h>
#include <Time.h>

_Bool trigger;

//kHz Sample 
ISR(TIMER0_COMPA_vect)
{
    TCNT0 = 0; //MUST RESET TIMER IMMEDIATELY AFTER INTERRUPT FOR ACCURATE TIMING
}


int main(void)
{
  DDRC = 0 << PORTC0; //A/D for PinC0
  DDRD = 0xFF;
  PORTB=0x00;

  uint8_t Rval;
  _Bool new_bit;
  
  //#############    ADC Setup    ###########################
  PRR = 0x00;
  ADCSRA = 1<<ADEN | 1<<ADPS2 | 1<<ADPS1 | 1<<ADPS0;
  ADMUX = (ADMUX & 0xF0) | 0<<REFS1 | 1<<REFS0 | 1<<ADLAR; //Set mux bits, for ADC0 single ended
  //#############################################################
  
  //################    TIMER SETUP   ###########################
  TCCR0A = 0x00;
  TCNT0 = 0x00; //pre-load timer to 0
  OCR0A = 0xF9; //for 1ms interrupt
  TIMSK0 = (1 << OCIE0A);
  sei();
  TCCR0B = 1<<CS21 | 1<<CS20; //TCCR0B = 0x03, pre-scaler 64;
  //#############################################################  

  
  while (1){
    if(trigger){
      ADCSRA |= (1<<ADSC);
      while ((ADCSRA & (1<<ADIF)) == 0);
      Rval = ADCH;
  
      if(Rval >= 0x80){
        new_bit = 1;
      }
      else{
        new_bit = 0;
      }
    }  
  }
}
