/*******************************************************
 * UART.C
 * UART functions for the GB110/flightworks combination
 *******************************************************/

// Include library for processor
#include <p24fj256gb110.h>

// Include the header for this file
#include "UART.h"

// Alias for the RTS Pin on UART1
#define TRTS_U1			TRISDbits.TRISD15

// Alias for the RTS Pin on UART2
#define TRTS_U2			TRISFbits.TRISF13

// Define a bitrate as 115200 Bd (BREGH=1)
#define BRATE			34

// Define the bits to enable a UART, set BREGH=1, 
// 1 stop, no parity + hardware flow control, and wake up enabled
#define U_ENABLE		0x8288

// This is the baud rate for the 2nd UART which talks
// to the Siemens instrument.  The instrument defaults
// to 19200 baud which should be fine.  For a 34MHz 
// clock, that turns out to be a BRATE of 103 (see the
// PIC24F Family Reference Manual for this calculations
#define BRATE_U2		207

// Define the bits to enable the second UART.
// This is the UART for talking to the Siemens instrument
// and should be BREGH=1, 1 stop, even parity, no
// flow control
#define U2_ENABLE		0x800A
//#define U2_ENABLE		0x8208

// Define the bits to enable a UART for transmission and clear all flags
#define U_TX			0x0400

// The function that initializes UART1 (115200@32MHz, 8, N, 1, CTS/RTS )
void initU1( void)
{
	U1BRG = BRATE;
	U1MODE = U_ENABLE;
	U1STA = U_TX;
    // Make sure RTS is an output
	TRTS_U1 = 0;
} // initU1

// send a character to serial port 1 through UART1
int putU1( int c)
{
	// Wait while the TX buffer is full
	// TODO kgomes I should put another timeout check here as this
	// will hang if it is trying to write something to a UART where
	// nobody is listening
	while ( U1STAbits.UTXBF);
	// Set character and send
	U1TXREG = c;
	// Return the same character
	return c;
} // putU1

// wait for a new character to arrive to the serial port
// attached to UART1
char getU1( void)
{
	// Wait for a new character to arrive or until a certain number
	// or cycles have passed
	unsigned long uart1TimeoutCounter = 0;
	unsigned long maxCount = 600000000;
	while ( (!U1STAbits.URXDA) && (uart1TimeoutCounter < maxCount)) {
		uart1TimeoutCounter++;
	}
	if (uart1TimeoutCounter >= maxCount)
		return 0x18;
	// Read the character from the receive buffer
	return U1RXREG;
}// getU1

// send a null terminated string to the serial port
// attached to UART1
void putsU1( char *s)
{
	// Loop until *s == '\0' which is the end of the string, send the
	// character and point to the next one.
	while( *s)
		putU1( *s++);
	// Send a terminating cr/line feed
//    putU1( '\r');
} // putsU1

// receive a null terminated string in a buffer of len char
// from the serial port attached to UART1
char *getsU1( char *s, int len)
{
	// Copy the buffer pointer
    char *p = s;
 
    do{
		// Wait for a new character
        *s = getU1();
		// Echo the character
		if (*s != 0x18)
	        putU1( *s);
        
		// Check for condition of backspace
        if (( *s==BACKSPACE)&&( s>p))
        {
			// Overwrite last character
            putU1( ' ');

			// Move the cursor
            putU1( BACKSPACE);

			// Add one to the length left to read
            len++;

			// Back the pointer down
            s--;
            continue;
        }

		// Check to see if it is a line feed and if so, ignore it
        if ( *s=='\n')
            continue;
		// Check for end of line, if so, end loop
        if ( *s=='\r')
            break;   
		
		// Check for the cancel character
		if (*s==0x18) {
			// Increment the buffer pointer but break
			s++;
			break;
		} 

		// Increment the buffer pointer      
        s++;

		// Decrement the size left to read
        len--;

    } while ( len>1 );      // until buffer full
 
	// Null terminate the string
    *s = '\0';              // null terminate the string 
    
	// Return the buffer pointer
    return p;

} // getsU1

// The function that initializes UART2 (19200@32MHz, 8, E, 1, NO FLOW CONTROL )
void initU2( void)
{
	U2BRG = BRATE_U2;
	U2MODE = U2_ENABLE;
	U2STA = U_TX;
    // Make sure RTS is an output
	TRTS_U2 = 0;
} // initU2

// send a character to serial port 2 through UART2
unsigned char putU2( unsigned char c)
{
	// Wait while the TX buffer is full
	while ( U2STAbits.UTXBF);
	// Set character and send
	U2TXREG = c;
	// Return the same character
	return c;
} // putU2

// A method that returns a zero if nothing has arrived
// on UART2 and a 1 if something has arrived.
int charArrivedAtUART2(void) {
	if (U2STAbits.URXDA) {
		return 1;
	} else {
		return 0;
	}
}

// wait for a new character to arrive to the serial port
// attached to UART2
unsigned char getU2( void)
{
	// Wait for a new character to arrive
	unsigned int i = 0;
	// NOTE: unsigned ints only go to 65535 so if you
	// make the condition anything over that it will
	// never bail out of this loop
	while ( (!U2STAbits.URXDA) && (i < 20000)) {
		i++;
	}
	// Read the character from the receive buffer
	return U2RXREG;
}// getU2

