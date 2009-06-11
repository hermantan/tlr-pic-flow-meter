/***********************************************************
 * UART.h
 * UART functions for the GB110/flighworks combination
 ***********************************************************/

// Define backspace character
#define BACKSPACE 0x8
            
// The function that initializes UART1 (115200@32MHz, 8, N, 1, CTS/RTS )
void initU1( void);

// send a character to serial port 1 through UART1
int putU1( int c);

// wait for a new character to arrive to the serial port
// attached to UART1
char getU1( void);

// send a null terminated string to the serial port
// attached to UART1
void putsU1( char *s);

// receive a null terminated string in a buffer of len char
// from the serial port attached to UART1
char * getsU1( char *s, int n);

// useful macros
#define clrscrU1() putsU1( "\x1b[2J") 
#define homeU1()   putsU1( "\x1b[1H") 
#define pcrU1()    putU1( '\r')

// The function that initializes UART2 (115200@32MHz, 8, N, 1, CTS/RTS)
// TODO kgomes - I think this needs to be Even parity with no flow control
void initU2( void);

// send a character to serial port 2 through UART2
unsigned char putU2( unsigned char c);

// A method that returns a zero if nothing has arrived
// on UART2 and a 1 if something has arrived.
int charArrivedAtUART2(void);

// wait for a new character to arrive to the serial port
// attached to UART2
unsigned char getU2( void);
