#include <avr/interrupt.h>
#include <Time.h>

#define USART_BAUDRATE 115200
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 8UL)))-1)

_Bool flushData;
_Bool trigger;
_Bool transmit;

//kHz Sample 
ISR(TIMER0_COMPA_vect)
{
    TCNT0 = 0; //MUST RESET TIMER IMMEDIATELY AFTER INTERRUPT FOR ACCURATE TIMING
    trigger = 1;
}

//Data Enable Interrupt; PIN D2
ISR(INT0_vect)
{
  transmit = ((PIND & 0x04)>>2);
  
  if (transmit){
    startTimer();
  } else {
    stopTimer();
    flushData = 1;
  }
  
  EIFR = EIFR | 1 << INTF0;     //Clear INT0 Flag
}

void TIMER_INIT(){
  TCCR0A = 0x00;
  OCR0A = 0xF9; //compare level for 1 millisecond 
}

void USART_INIT() {
  UBRR0 = BAUD_PRESCALE;
  /*
    1 stop bit
    8-bit characters
    Asynchronous mode
  */
  UCSR0C = ((0 << USBS0)) |
           (1 << UCSZ01) | (1 << UCSZ00) |
           (0 << UMSEL01) | (0 << UMSEL00);
  UCSR0B = ((1 << RXEN0) | (1 << TXEN0)); // Enable receiver and transmitter
  UCSR0A = (1 << U2X0); // Enable double asynch mode
}

void startTimer(){
  TCNT0 = 0x7C; //pre-load timer to 124, 
  TCCR0B = 1 <<CS21 | 1<<CS20; //TCCR0B = 0x03, pre-scaler 64;
  TIMSK0 = (1 << OCIE0A);
}

void stopTimer(){
  TIMSK0 = (0 << OCIE0A);
  TCCR0B = 0x00;
}

void USART_write_char(uint8_t data) {
  //while the transmit buffer is not empty loop
  while (!(UCSR0A & (1 << UDRE0)));

  //when the buffer is empty write data to the transmitted
  UDR0 = data;
}


int main(void)
{
  DDRC = 0xFF;  //data on C0
  DDRB=0x0F;    //trigger snoop on B0
  uint8_t data_buffer[1024];
  transmit = 0;
  flushData = 0;
  _Bool new_bit;
  uint32_t byte_counter = 0;
  uint32_t bit_counter = 0;
  uint32_t i = 0;
  uint8_t current_byte = 0x00;
  uint32_t test_bit = 0x00;

  //###############   PIN INTERRRUPT SETUP  #####################
  DDRD = DDRD | (0<<PORTD2)| (0<<PORTD3);  //D2 Interrupt set to input
  EICRA = 0<<ISC01 | 1<<ISC00; //set INT0 to trigger on logic change [0 1]
  EIMSK = 1<<INT0; // enable INT0
  sei();
  //#################################################################

  TIMER_INIT();
  USART_INIT();
  
  while(1){
    // Wait for trigger time
    PORTB = trigger;
    //PORTC = flushData;
    if(trigger){
      trigger = 0;

      // Get bit from input pin and shift it into the byte
      new_bit = (PIND & 0x08) >> 3; 
      // PORTC = new_bit; 
      current_byte |= (new_bit << bit_counter);
      bit_counter++;
      
      if (bit_counter == 8) {
        // Bit counter is maxed, store byte in buffer, reset byte, reset bit counter
        data_buffer[byte_counter] = current_byte;
        byte_counter++;
        current_byte = 0x00;
        bit_counter = 0;
      }
    }  

    // Wait to flush the data to serial
    if(flushData) {
      cli();
      // Let PC app know we're sending data
      USART_write_char('D');
      
      // Write each element in buffer to serial, erasing buffered location
      for(i = 0; i < 1024; i++) {
        PORTC=test_bit;
        test_bit ^= test_bit; 
        USART_write_char(data_buffer[i]);
        data_buffer[i] = 0x00;
      }
      // Reset buffer position counter
      byte_counter = 0;
      flushData = 0;
      sei();
    }
  }
}
