#include <avr/io.h>
#include <inttypes.h>
#include <avr/interrupt.h>

void USARTInit(void);
char USARTReadChar(void);
void USARTWriteChar(char);
void USART_TransmitString(char *);


void USART_TransmitString(char *str) {
  while(*str != 0x00) {
    USARTWriteChar(*str);
    str++;
  }
}

void USARTInit(void)
{
  /*
  First, initialize the USART with baud rate = 19200bps
  For Baud rate = 19200bps UBRR value = 51
  */
  // uint16_t baudrateValue = 51;

  /*
  First, initialize the USART with baud rate = 115200 bps
  For Baud rate = 115200 bps UBRR value = 8
  */
  unsigned int baudrateValue = 8;

  UBRR0L = baudrateValue;
  UBRR0H = (baudrateValue >> 8);

  /*Set Frame Format
  >> Asynchronous mode
  >> No Parity
  >> 1 StopBit
  >> char size 8
  */
  UCSR0C=0x03;
   
  UCSR0B = (1 << RXEN0) | (1 << TXEN0); //Enable the receiver and transmitter
  UCSR0B |= (1 << RXCIE0);

  UCSR0A &= ~(1 << U2X0); // Clear auto-asynch double time bit
  sei(); // enable global interrupts
}


/// This function is used to read the available data from USART
char USARTReadChar()
{
  //Wait until data is received in the buffer
  while(!(UCSR0A & (1 << RXC0)));

  // Return the value in the buffer
  return UDR0;
}


//This function writes the given "data" to the USART
void USARTWriteChar(char data)
{
  //Wait until the transmitter is ready
  while(!(UCSR0A & (1<<UDRE0)));

  //Now write the data to USART buffer
  UDR0 = data;
}

ISR(USART_RX_vect) {
  unsigned char data = USARTReadChar();

  USART_TransmitString("\nDATA RECEIVED: " + data);
}

int main() {
  USARTInit();
  while(1);
}
