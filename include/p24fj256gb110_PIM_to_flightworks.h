/********1*********2*********3*********4*********5*********6*********7**********
*
* 	                          p24fj256gb110_PIM_to_flightworks.h
*
*  This header file defines functions that are used to setup the Peripheral Pin
*  Select (PPS) so the GB110 microcontroller on a Plug In Module (PIM) will be
*  setup correctly to enable the use of the UART's, SD card, and other 
*  peripherals.
*
*  History:
*    10/28/08 - Took the mappings done by Wayne Duquaine for mapping the GB110
*               PIM to the Explorer 16 and refactored to match the use of the 
*               GB110 on the Flightworks AV32 board                  Kevin Gomes
*******************************************************************************/

// The function to initialize the mappings
// for the GB110 on the AV32 board
void  init_PPS(void);

// GB110 ERRATA
#if defined(__PIC24FJ256GB110__)
#define EEPROM_SPI_ENSURE_CLOCK_DONE  while(PORTGbits.RG6==1);
#else
#define EEPROM_SPI_ENSURE_CLOCK_DONE
#endif
