/********1*********2*********3*********4*********5*********6*********7**********
*
* 	                          p24fj256gb110_PIM_to_flightworks.c
*
*  This code implements the functions that are used to setup the Peripheral Pin
*  Select (PPS) so the GB110 microcontroller on a Plug In Module (PIM) will be
*  setup correctly to enable the use of the UART's, SD card, and other
*  peripherals
*
*  PLEASE NOTE:
*  Some of these mappings may seem a bit strange, but keep in mind that the PIM 
*  pins on the flightworks board do not directly map to the pins on the GB110.  
*  This is due to the fact that Microchip re-routed some pins on the PIM for 
*  the Explorer 16 board.  Please see the PIC24FJ156GB110 PIM Manual for details.
*
*  History:
*    10/28/08 - Took the mappings done by Wayne Duquaine for mapping the GB110
*               PIM to the Explorer 16 and refactored to match the use of the 
*               GB110 on the Flightworks AV32 board                  Kevin Gomes
*******************************************************************************/
// Include the library for the GB110
#include <p24FJ256GB110.h>

// GB110 Errata
#define SPI2_ENSURE_CLOCK_DONE  while(PORTGbits.RG6==1);

// In order to set the output pins, we need to find the number for the
// peripherals that we are going to map. In table 9-2 of the GB110 
// manual, we find those numbers (page 125).  These are those numbers
// for the peripherals that we will need to map
#define U1TX_IO 		3
#define U1RTS_IO		4
#define U2TX_IO			5
#define U2RTS_IO		6
#define SDO1_IO			7
#define SCK1OUT_IO		8

/**********************************************************************
 * init_PPS function
 *
 * This function unlocks the Peripheral Pin Select (PPS) configuration,
 * changes the pin mappings, and then re-locks the PPS configuration.
 **********************************************************************/
void  init_PPS (void)
{

// Make sure the processor is defined as the GB110
#if defined(__PIC24FJ256GB110__)

	// Unlock the PPS configuration
	asm volatile ( "MOV   #OSCCON,w1 \n"
                   "MOV   #0x46,w2 \n"
                   "MOV   #0x57,w3 \n"
                   "MOV.b w2,[w1]  \n"
                   "MOV.b w3,[w1]  \n"
                   "BCLR  OSCCON,#6");

	// ********************************************************
	// Timer1 Input Pin setup to point to pin 74 where external
	// 32 kHz oscillator is located
	// ********************************************************
	// T1CK  = RPI37  (which is pin 74, which is RC14)
	//RPINR2bits.T1CKR   = 37;

	// ******************************************************
    // UART1 Pin Mappings
    // ******************************************************
    // Input pins
	// Set UART1's RX to RPI41 (which is pin 9, which is RC4)
	RPINR18bits.U1RXR = 41;
	// Set UART1's CTS to RPI43 (which is pin 47, which is RD14)
	RPINR18bits.U1CTSR = 43;

    // Output pins
	// Set UART1's TX to RP30 (which is pin ?, which is ?)
	RPOR15bits.RP30R = U1TX_IO;
	// Set UART1's RTS to RP5 (which is pin 48, which is RD15)
	RPOR2bits.RP5R = U1RTS_IO;

	// ******************************************************
	// UART2 Pin Mappings
	// ******************************************************
	// Input pins
	// Set UART2's RX to RP10 (which is pin 49, which is RF4)
	RPINR19bits.U2RXR = 10;
	// Set UART2's CTS to RPI32 (which is pin 40, which is RF12)
	RPINR19bits.U2CTSR = 32;

	// Output pins
	// Set UART2's TX to RP17 (which is pin 50, which is RF5)
	RPOR8bits.RP17R = U2TX_IO;
	// Set UART2's RTS to RP31 (which is pin 39, which is RF13)
	RPOR15bits.RP31R = U2RTS_IO;

	// ******************************************************
	// SPI1 Mappings
	// ******************************************************
	// Input pins
	// Set the SDI1 to RP23 (which is pin 77, which is RD2)
	RPINR20bits.SDI1R = 23;
	
	// Output pins
	// Set SCK1OUT to RP0 (which is pin 25, which is RB0)
	RPOR0bits.RP0R = SCK1OUT_IO;
	// Set the SDO1 to RP15 (which is pin 53, which is RF8)
	RPOR7bits.RP15R = SDO1_IO;

	// re-Lock the PPS configuration
    asm volatile ( "mov #OSCCON,w1 \n"
                   "mov #0x46, w2 \n"
                   "mov #0x57, w3 \n"
                   "mov.b w2,[w1] \n"
                   "mov.b w3,[w1] \n"
                   "bset OSCCON, #6");
#else
	// Don't do anything if it is not the GB110 processor
#endif
}
