#include<avr/io.h>
#include<avr/interrupt.h>

// Necessary Definitions
#define F_CPU 16000000
#define USART_BAUDRATE 115200
#define BAUD_PRESCALE (((F_CPU / (USART_BAUDRATE * 8UL)))-1)
#define PACKET_SIZE 1024


// Define packet structures
struct initializer_t {
  uint32_t numPackets;
  uint8_t filename[32];
  uint8_t dataHash[32];
};

struct packet_t {
  uint32_t packetID;
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
  uint8_t integerBuffer[4];
  uint32_t temp;

  struct initializer_t initializerPacket;
  struct packet_t packet;
  int bufferCount = 0;

  while(1){
    char character = USART_receive_char();
    uint32_t dataSize = 0, i, j;
    
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


      // Print number of packets, the filename, and the hash
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

        // Read in the packet ID
        for(j = 0; j < 4; j++){
          integerBuffer[j] = USART_receive_char();
        }
        packet.packetID = *(uint32_t *)integerBuffer;
        
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
        for(j = 0; j < packet.packetID; j++){
          USART_write_char('I');
        }
        for(j=0; j < packet.numBytes; j++){
          USART_write_char('B');
        }
      }
      
      
      USART_write_string("ER");
    }
  }
}
