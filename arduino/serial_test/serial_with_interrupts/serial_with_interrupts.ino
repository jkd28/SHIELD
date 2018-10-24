#include<avr/io.h>
#include<avr/interrupt.h>

// Necessary Definitions
#define F_CPU 16000000
#define USART_BAUDRATE 115200
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 8UL)))-1)
#define BUFFER_SIZE 32

// Some global definitions
char stringBuf[BUFFER_SIZE];
int bufferCount = 0;

void flush_buffer(){
  int i;
  for(i = 0; i < BUFFER_SIZE; i++){
    if (stringBuf[i] != 0x00){
      USART_send(stringBuf[i]);
      stringBuf[i] = 0x00;  
    }
  }
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

void USART_send(unsigned char data) {
  //while the transmit buffer is not empty loop
  while (!(UCSR0A & (1 << UDRE0)));

  //when the buffer is empty write data to the transmitted
  UDR0 = data;
}

unsigned char USART_receive(void) {
  /* Wait for data to be received */
  while (!(UCSR0A & (1 << RXC0)));

  /* Get and return received data from buffer */
  return UDR0;
}

void USART_sendString(char *str) {
  int i = 0;
  while(str[i] != 0x00) {
    USART_send(str[i++]);
  }
  return;
}

/* TODO Perfect this code */
int main() {
  USART_INIT();

  while(1){
    char character = USART_receive();
    // Check for START bit
    if (character == 'S') {
      USART_sendString("RS");
      character = USART_receive();

      // Check for END bit
      while (character != 'E'){
        // Read each character before the end bit into the buffer
        character = USART_receive();
        stringBuf[bufferCount++] = character;
      }
      USART_sendString("RE");
    }
  }
}
