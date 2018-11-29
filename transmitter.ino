#include<avr/io.h>
#include<avr/interrupt.h>

// Necessary Definitions
#define F_CPU 16000000
#define USART_BAUDRATE 115200
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 8UL)))-1)
#define PACKET_SIZE 1024
#define LOW_BIT 0x34  // 36kHz
#define HIGH_BIT 0x2E // 42kHz
#define NO_BIT 0x3F   // 32kHz

// Define some globals
_Bool trigger = 0;

// Define packet structures
struct initializer_t {
  uint32_t numPackets;
  uint8_t filename[32];
  uint8_t dataHash[32];
};

struct packet_t {
  uint32_t numBytes;
  uint8_t data[1024];
};

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

void PWM_INIT() {
  DDRD = DDRD | (1<<PIND3); //analog pin out: 3 is pwm
  OCR2B = 0x00; //set duty to 0%
  DDRC = 0x01;  //trigger snoop on PORTC0
  DDRB = 0xFF; //output for data snoop is PORTB

  //TCCR2A = _BV(COM2A1) | _BV(COM2B1) | _BV(WGM21) | _BV(WGM20);
  TCCR2A = _BV(COM2B1) | _BV(WGM21) | _BV(WGM20); //com2b1 enables pwm on analog 3, wgm bits set pwm type
  TCCR2B = 1 << WGM22 | 0 << CS22 | 1 << CS21 | 0 << CS20; //CS bits for pre-scale of 8
  OCR2B = 0x1F; //start pwm
  OCR2A = NO_BIT;
}

void TIMER_INIT(){
  TCCR0A = 0x00;
  TCNT0 = 0x00; //pre-load timer to 0
  OCR0A = 0xF9; //compare level for 1 millisecond
  TIMSK0 = (1 << OCIE0A);
}

void startTimer(){
  TCCR0B = 1 <<CS21 | 1<<CS20; //TCCR0B = 0x03, pre-scaler 64;
  TCNT0 = 0;
  sei();
}

void stopTimer(){
  cli();
  OCR2A = NO_BIT;
}

ISR(TIMER0_COMPA_vect)
{
  TCNT0 = 0;
  trigger = 1;
}

unsigned char USART_receive_char(void) {
  /* Wait for data to be received */
  while (!(UCSR0A & (1 << RXC0)));

  /* Get and return received data from buffer */
  return UDR0;
}

void USART_write_char(uint8_t data) {
  //while the transmit buffer is not empty loop
  while (!(UCSR0A & (1 << UDRE0)));

  //when the buffer is empty write data to the transmitted
  UDR0 = data;
}

void USART_write_string(uint8_t *str) {
  int i = 0;
  while(str[i] != 0x00) {
    USART_write_char(str[i++]);
  }
  return;
}

int main() {
  USART_INIT();
  PWM_INIT();
  TIMER_INIT();

  while(1){
    uint8_t integerBuffer[4];
    struct initializer_t initializerPacket;
    struct packet_t packet;
    uint32_t sendCounter = 0;
    uint32_t dataSize = 0, i, j;
    _Bool data_bit = 0;
    uint8_t bit_counter = 0;

    char character = USART_receive_char();
    // Check for START bit
    if (character == 'S') {
      USART_write_string("RS");

      // Now read intitializer packet
      for (i = 0; i < 32; i++){
          initializerPacket.filename[i] = USART_receive_char();
      }
      for (i = 0; i < 32; i++){
          initializerPacket.dataHash[i] = USART_receive_char();
      }
      for (i = 0; i < 4; i++){
          integerBuffer[i] = USART_receive_char();
      }
      initializerPacket.numPackets = *(uint32_t *)integerBuffer; // Cast the array to a useable uint32_t


      // Print number of packets, the filename, and the hash for DEBUG
      for(i = 0 ; i < 4; i++){
        USART_write_char(integerBuffer[i]);
      }
      for(i = 0 ; i < 32; i++){
        USART_write_char(initializerPacket.filename[i]);
      }
      for(i = 0 ; i < 32; i++){
        USART_write_char(initializerPacket.dataHash[i]);
      }


      // Request a packet and transmit the packet until we've done all packets
      for (i = 0; i < initializerPacket.numPackets; i++){
        // Request a packet
        USART_write_string("PR");

        // Read in the number of bytes
        for(j = 0; j < 4; j++){
          integerBuffer[j] = USART_receive_char();
        }
        packet.numBytes = *(uint32_t *)integerBuffer;

        // Read in the data
        for(j = 0; j < packet.numBytes; j++){
          packet.data[j] = USART_receive_char();
        }

        // Print some debug stuff
        for(j=0; j < packet.numBytes; j++){
          USART_write_char('B');
        }

        // SEND SOME DATA
        sendCounter = 0;
        data_bit = 0;
        bit_counter = 0;
        startTimer();
        while(sendCounter < packet.numBytes) {
          PORTC = trigger;
          if(trigger){
            trigger = 0;

            // Pick our bit from the packet
            if(bit_counter < 8){
              data_bit = ((packet.data[sendCounter] >> bit_counter) & 0x1);
            } else if(sendCounter < packet.numBytes - 1) {
              bit_counter = 0;
              sendCounter++;
              data_bit = (packet.data[sendCounter] & 0x1);
            } else if(sendCounter == packet.numBytes - 1) {
                sendCounter++;
                bit_counter = 0;
                continue;
            }
            bit_counter++;

            // Set frequency according to bit value
            if(data_bit){
              OCR2A = HIGH_BIT;
              PORTB = 0xFF;
            } else {
              OCR2A = LOW_BIT;
              PORTB = 0x00;
            }
          }
        }
        stopTimer();
      }
      USART_write_string("ER");
    }
  }
}
