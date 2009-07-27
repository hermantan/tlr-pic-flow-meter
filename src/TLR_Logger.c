/***************************************************************************************
 * This is the main routine for the TLR_Logger program.
 *
 * History:
 *    10/28/08 - Started from scratch basically.  I moved to the flightworks board and
 *               now have a better understanding of things.                   Kevin Gomes
 ****************************************************************************************/

// Include the PIC24FJ256GB110 Header
#include <p24fj256gb110.h>

// Include the functions that do the mapping from the 
// PIC24FJ256G110 on a PIM to the Flightworks support
// board
#include "p24fj256gb110_PIM_to_flightworks.h"

// Include the UART library for comms to flow meter and terminal
#include "UART.h"

// Include Microchips SD File Library
#include "FSIO.h"

// Include the library for modbus functionality to flow meter
#include "modbus.h"

// Include the string library
#include <string.h>

// Define a constant to represent the bit mask for turning on the 96MHZ
#define PLL_96MHZ_ON	0xF7FF

// Define the number of samples to average
#define NUMBER_OF_SAMPLES_TO_AVERAGE 	4

// Configure the PIC24FJ256GB110, turn JTAG off, watchdog off
_CONFIG1(ICS_PGx2 & JTAGEN_OFF & FWDTEN_OFF)
// ?
_CONFIG2(FNOSC_PRIPLL & PLL_96MHZ_ON & PLLDIV_DIV2 & OSCIOFNC_OFF & FCKSM_CSDCMD & POSCMOD_XT)

// A flag to indicate that the UART 1 is active (terminal session is active)
int terminalActive = 0;

// The interrupt service routine for the RTCC
void _ISR _RTCCInterrupt(void) {
	// Clear the interrupt flag
	_RTCIF = 0;

	// Re-enable the alarm (the alarm flag gets cleared when alarm fires
	ALCFGRPTbits.ALRMEN = 1;	// Enable the Alarm
}

// The Interrupt service routine for the UART receiver
void _ISR _U1RXInterrupt(void) {
	// Clear the interrupt flag
	_U1RXIF = 0;

	// If the terminal is not active, print a message
	if (terminalActive == 0) {
		// Write message
		clrscrU1();
		homeU1();
		putsU1("                                          \r");
		putsU1("Waking up, please wait for TLR> prompt ...\r");
	}

	// Mark that the terminal session is active
	terminalActive = 1;	
}

// This function sets up the various peripherals that are associated with the PIC controller
void setupPeripherals() {
	// Shut off the USB
	PMD4bits.USB1MD = 1;

	// Disable all the I2C peripherals
	I2C1CONbits.I2CEN = 0;
	I2C2CONbits.I2CEN = 0;
	I2C3CONbits.I2CEN = 0;
	// Turn off all the I2C peripherals
	PMD1bits.I2C1MD = 1;
	PMD3bits.I2C2MD = 1;
	PMD3bits.I2C3MD = 1;

	// Disable SPI 2 and 3
	SPI2STATbits.SPIEN = 0;
	SPI3STATbits.SPIEN = 0;

	// Turn off SPI 2 and 3
	PMD1bits.SPI2MD = 1;
	PMD6bits.SPI3MD = 1;

	// Turn off UARTs 3 and 4
	PMD3bits.U3MD = 1;
	PMD4bits.U4MD = 1;

	// Turn off the PMP port
	PMD3bits.PMPMD = 1;

	// Turn off the Analog to Digital Converter
	PMD1bits.ADC1MD = 1;

	// Turn off all Input Captures
	PMD2bits.IC1MD = 1;
	PMD2bits.IC2MD = 1;
	PMD2bits.IC3MD = 1;
	PMD2bits.IC4MD = 1;
	PMD2bits.IC5MD = 1;
	PMD2bits.IC6MD = 1;
	PMD2bits.IC7MD = 1;
	PMD2bits.IC8MD = 1;
	PMD5bits.IC9MD = 1;

	// Turn off all Output Comparators
	PMD2bits.OC1MD = 1;
	PMD2bits.OC2MD = 1;
	PMD2bits.OC3MD = 1;
	PMD2bits.OC4MD = 1;
	PMD2bits.OC5MD = 1;
	PMD2bits.OC6MD = 1;
	PMD2bits.OC7MD = 1;
	PMD2bits.OC8MD = 1;
	PMD5bits.OC9MD = 1;

	// Turn off CMP
	PMD3bits.CMPMD = 1;

	// Turn off CRC
	PMD3bits.CRCMD = 1;

	// Turn off UPWM
	PMD4bits.UPWMMD = 1;

	// Turn off REFO
	PMD4bits.REFOMD = 1;

	// Turn off CTMU
	PMD4bits.CTMUMD = 1;

	// Turn off LVD
	PMD4bits.LVDMD = 1;

	// Turn off Timers 1-5
	PMD1bits.T1MD = 1;
	PMD1bits.T2MD = 1;
	PMD1bits.T3MD = 1;
	PMD1bits.T4MD = 1;
	PMD1bits.T5MD = 1;

	// Start off with SPI 1 turned off
	PMD1bits.SPI1MD = 1;
}

// This is the function to read in all the data from the flow meter and record
// in the data buffer provided to the method call
void readAndLogSample(void) {
	// Make sure SPI1 is enabled
	PMD1bits.SPI1MD = 0;

	// The buffer to use to write to the file
	char logRecordBuffer[255];

	// Clear out the buffer
	int p = 0;
	for (p = 0; p < 255; p++) {
		logRecordBuffer[p] = '\0';
	}

	// First we send a command just to clear up and establish comms.  We then
	// throw away that response.  This is more of a work around, but it seemed
	// like every time it would wake up
	unsigned char dateAndTime[6];
	readActualDateAndTime(dateAndTime);

	// Read flow values and average
	float averageFlow = 0;
	char i = 0;
	for (i=0; i<NUMBER_OF_SAMPLES_TO_AVERAGE; i++) {
		averageFlow += readFlowRate();
	}
	averageFlow = averageFlow/NUMBER_OF_SAMPLES_TO_AVERAGE;

	// Read Totalizer Integer
	long integerTotalizerOne = readTotalizer1Integer();

	// Read the transmitter temp
	float transTemp = readTransmitterTemp();

	// Read battery capacity
	unsigned char battCap = readBatteryCapacity();

	// Power status
	unsigned char powerStat = readPowerStatus();

	// Read the fault status
	unsigned int faultStatus = readFaultStatus();

	// Grab the current time and date and put in RTCC register
	RTCCgrab();

	// Initialize the File system
	if (FSInit()){
		// Define a pointer to the log file
		FSFILE *logFile;

		// A variable to keep track of how many characters were written 
		int charsWritten;

		// Define the name of the log file
		char logFileName[] = "DATALOG.TXT";

		// The mode to open the file in (a = append, w = write/over-write
		char appendArg[] = "a";

		// Open the file
		logFile = FSfopen(logFileName,appendArg);

		// If the file opened OK, write the log entry
		if (logFile != NULL) {
			// Write to a buffer first
			charsWritten = sprintf(logRecordBuffer,"20%02u-%02u-%02uT%02u:%02u:%02u,%3.3f,%ld,%2.2f,%3u,%1u,%2o\n", 
				getYear(), getMonth(), getDay(), getHour(), getMin(), getSec(), 
				averageFlow, integerTotalizerOne,transTemp,battCap,powerStat,faultStatus);
			// Write those to a file
			FSfwrite(logRecordBuffer,1,charsWritten,logFile);

			// Close the file
			FSfclose(logFile);
		}
	}

	// Now shutdown SPI1
	PMD1bits.SPI1MD = 1;
}

// The main program
int main(void) {

	// First, run the fuction that will perform the Peripheral Pin Select (PPS)
	// to make sure the microcontroller is setup correctly and is connected to
	// the correct pins that route to the various peripherals
	init_PPS();

	// Initialize the Real Time Clock and Calendar
	RTCCInit();
	
	// Set the alarm register seconds, minutes, hours, days and write to Alarm
	RTCCSetBinSec(0,1);
	RTCCSetBinMin(0,1);
	RTCCSetBinHour(5,1);
	RTCCAlarmSet();

	// Set the alarm interval
	// 0x0 = Every half second
	// 0x1 = Every second
	// 0x2 = Every 10 seconds
	// 0x3 = Every minute
	// 0x4 = Every 10 minutes
	// 0x5 = Every hour
	// 0x6 = Once a day
	// 0x7 = Once a week
	// 0x8 = Once a month
	// 0x9 = Once a year
	ALCFGRPTbits.AMASK = 0x6;

	// Enable the alarm
	ALCFGRPTbits.ALRMEN = 1;

	// Initialize the interrupt for the RTCC
  	// Set the interrupt priority to level 4.  This is the default, but set 
	// priority explicitly anyway
	_RTCIP = 4;
	// Clear the interrupt flag (0) so it will not fire when enabled
	_RTCIF = 0;
	// Enbale (1) the interrupt
	_RTCIE = 1;


	// Setup the interrupt for the receive on UART1 so terminal can wake
	// up processor from sleep
	// Set the priority to 4 (default, but set anyway)
	_U1RXIP = 4;
	// Clear the flag (0) that says an interrupt has occurred
	_U1RXIF = 0;
	// Enable (1) the interrupt
	_U1RXIE = 1;
	
	// Set the default interrupt priority level of the processor
	// to 0.  This is the default, but explicity set it anyway
	_IPL = 0;

	// Initialize the first UART (UART1) for terminal comms
	initU1();

	// Initialize the second UART (UART2)
	initU2();

	// Setup the rest of the peripherals
	setupPeripherals();

	// Enter an endless loop
	while(1) {

		// Check to see if the terminal session is active
		if (terminalActive > 0) {
			// Since the terminal is active, we assume the user wants to do something

			// This is a variable that keeps track of whether or not the user wants to exit and
			// go back to normal mode.
			int userExit = 0;

			// Enter loop that asks for user's input and processes it until the user
			// exits.
			putsU1("\r");
			putsU1("                                                          \r");
			putsU1("Welcome! All normal processing is halted until 'resume' is entered\r");

			// The buffer for the user's command
			char command[128];

			// The buffer to use to print message to the terminal
			char toPrint[255];

			// Loop until the user wants to exit
			while(userExit == 0) {

				// Print out the prompt
				putsU1("TLR>");

				// Clear the command buffer
				int comCtr = 0;
				for (comCtr = 0; comCtr < 128; comCtr++) {
					command[comCtr] = '\0';
				}

				// Clear the terminal message buffer
				int p = 0;
				for (p = 0; p < 255; p++) {
					toPrint[p] = '\0';
				}

				// Read the command from the user
				getsU1(command, 128);

				// Check the return string for the 'Cancel' character 0x18 (^X)
				// which means the get command from user most likely timed out
				char cancelChar[2] = {0x18,'\0'};
				char * cancelLocation = strstr(command,cancelChar);
				if (cancelLocation != '\0') {
					userExit = 1;
					terminalActive = 0;
					break;
				}

				// Check to see if the command was 'resume'
				if (strncmp(command,"resume",6) == 0) {
					userExit = 1;
					// Clear the terminal active flag
					terminalActive = 0;
					putsU1("Exited terminal and resuming normal operation.\r");
				} else if (strncmp(command,"gpdt",4) == 0) {
					// The user has request the date and time from the PIC
					RTCCgrab();
					sprintf(toPrint,"PIC Time = 20%02u-%02u-%02uT%02u:%02u:%02u \r", 
						getYear(), getMonth(), getDay(), getHour(), getMin(), getSec());
				} else if (strncmp(command,"gplf",4) == 0) {
					// The user has requested a dump of the contents of the log file
					// Turn on SPI1
					PMD1bits.SPI1MD = 0;
					// Initialize the File system
					if (FSInit()){
						// Define a pointer to the log file
						FSFILE *logFile;

						// Define the name of the log file
						char logFileName[] = "DATALOG.TXT";

						// The mode to open the file in (a = append, w = write/over-write, r = read-only)
						char appendArg[] = "r";

						// Open the file
						logFile = FSfopen(logFileName,appendArg);

						// If the file opened OK, loop over the lines and print to the terminal
						if (logFile != NULL) {
							// While not at end of file
							unsigned char fromFile[1];
							while(!FSfeof(logFile)) {
								FSfread(fromFile,1,1,logFile);
								if (fromFile[0] == '\n'){ 
									putU1('\r');
								} else {
									putU1(fromFile[0]);
								}
							}
						}
						// Close the file
						FSfclose(logFile);
					}
					// Write done message to terminal buffer
					sprintf(toPrint,"Done reading log file\r");
					// Turn off SPI1
					PMD1bits.SPI1MD = 1;
				} else if (strncmp(command,"spyr",4) == 0) {
					// Prompt for year
					putsU1("Enter last two digits of the year: i.e. '08' for 2008\r> ");
					getsU1(command,128);
					// Set the year using the entered two digits
					char yearAsChar[3] = {command[0],command[1],'\0'};
					// Convert to int
					int yearAsInt = atoi(yearAsChar);
					// Grab the current times to refresh the holding variables
					RTCCgrab();
					// Change the year holding variable
					RTCCSetBinYear(yearAsInt,0);
					// Write to the RTCC registers
					RTCCSet();
					// Read the clock again
					RTCCgrab();
					// Set the response buffer
					sprintf(toPrint,"PIC Time = 20%02u-%02u-%02uT%02u:%02u:%02u \r", 
						getYear(), getMonth(), getDay(), getHour(), getMin(), getSec());
				} else if (strncmp(command,"spmo",4) == 0) {
					// Prompt for month
					putsU1("Enter month in full two digits: i.e. '08' for August\r> ");
					getsU1(command,128);
					// Set the month using the response from the user
					char monthAsChar[3] = {command[0],command[1],'\0'};
					// Convert to int
					int monthAsInt = atoi(monthAsChar);
					// Grab the current time from RTCC and put in holding variables
					RTCCgrab();
					// Update holding variables
					RTCCSetBinMonth(monthAsInt,0);
					// Set the RTCC from the holding variables
					RTCCSet();
					// Grab the current time again and write to terminal buffer
					RTCCgrab();
					sprintf(toPrint,"PIC Time = 20%02u-%02u-%02uT%02u:%02u:%02u \r", 
						getYear(), getMonth(), getDay(), getHour(), getMin(), getSec());
				} else if (strncmp(command,"spdy",4) == 0) {
					// Prompt for day
					putsU1("Enter day of month in full two digits: i.e. '02' for the 2nd\r> ");
					getsU1(command,128);
					// Set the day using the first two chars of the response 
					char dayAsChar[3] = {command[0],command[1],'\0'};
					// Convert to int
					int dayAsInt = atoi(dayAsChar);
					// Grab the current time from RTCC and set holding variables
					RTCCgrab();
					// Update holding variables
					RTCCSetBinDay(dayAsInt,0);
					// Set the RTCC from the holding variables
					RTCCSet();
					// Read back from the RTCC and set terminal buffer
					RTCCgrab();
					sprintf(toPrint,"PIC Time = 20%02u-%02u-%02uT%02u:%02u:%02u \r", 
						getYear(), getMonth(), getDay(), getHour(), getMin(), getSec());
				} else if (strncmp(command,"sphr",4) == 0) {
					// Prompt for hour
					putsU1("Enter hour of day in full two digits (24 hour): i.e. '02' for 2AM\r> ");
					getsU1(command,128);
					// Set the hour using the first two chars of the response 
					char hourAsChar[3] = {command[0],command[1],'\0'};
					// Convert to an integer
					int hourAsInt = atoi(hourAsChar);
					// Grab the current time and put in holding variables
					RTCCgrab();
					// Set the hour on the holding variables
					RTCCSetBinHour(hourAsInt,0);
					// Write the holding variables to the RTCC
					RTCCSet();
					// Read the time again and put in terminal buffer
					RTCCgrab();
					sprintf(toPrint,"Current Time = 20%02u-%02u-%02uT%02u:%02u:%02u \r", 
						 getYear(), getMonth(), getDay(), getHour(), getMin(), getSec());
				} else if (strncmp(command,"spmn",4) == 0) {
					// Prompt for minute
					putsU1("Enter minute in full two digits: i.e. '02' for 2 minutes after hour\r> ");
					getsU1(command,128);
					// Set the minute using the first two chars of the response 
					char minuteAsChar[3] = {command[0],command[1],'\0'};
					// Convert to an integer
					int minuteAsInt = atoi(minuteAsChar);
					// Grab the current time and put in holding variables
					RTCCgrab();
					// Write the minute to the holding variables
					RTCCSetBinMin(minuteAsInt,0);
					// Set the RTCC with values in the holding variables
					RTCCSet();
					// Read the RTCC again and put in terminal buffer
					RTCCgrab();
					sprintf(toPrint,"Current Time = 20%02u-%02u-%02uT%02u:%02u:%02u \r", 
						getYear(), getMonth(), getDay(), getHour(), getMin(), getSec());
				} else if (strncmp(command,"spsc",4) == 0) {
					// Prompt for seconds
					putsU1("Enter seconds in full two digits: i.e. '02' for 2 seconds after minute\r>");
					getsU1(command,128);
					// Set the second using the first two chars of the response 
					char secondAsChar[3] = {command[0],command[1],'\0'};
					// Convert to an integer
					int secondAsInt = atoi(secondAsChar);
					// Read the current time and put in holding variables
					RTCCgrab();
					// Update the seconds on the holding variables
					RTCCSetBinSec(secondAsInt,0);
					// Set the RTCC with values from the holding variables
					RTCCSet();
					// Read the time again and put in terminal buffer
					RTCCgrab();
					sprintf(toPrint,"Current Time = 20%02u-%02u-%02uT%02u:%02u:%02u \r", 
						getYear(), getMonth(), getDay(), getHour(), getMin(), getSec());
				} else if (strncmp(command,"ptsm",4) == 0) {
					// Ask the user how many samples to take
					putsU1("How many samples do you want to log (01-99)?\r");
					putsU1("Enter in two digit format (i.e. 02 for two samples)\r>");
					getsU1(command,128);
					// Grab the user's reply
					char numSampsAsChar[3] = {command[0],command[1],'\0'};
					// Convert to an integer
					int numberOfSamplesToTake = atoi(numSampsAsChar);
					putsU1("OK, starting to sample");
					int lc;
					for (lc = 0; lc < numberOfSamplesToTake; lc++) {
						// Read the log sample
						readAndLogSample();
						putU1('.');
					}
					sprintf(toPrint,"OK, done sampling, use gplf to see the samples. \r");
				} else if (strncmp(command,"pssi",4) == 0) {
					putsU1("Choose interval that the PIC will sample the flow meter:\r");
					putsU1("A = Every 10 minutes\rB = Every hour\rC = Once a day\rD = Once a week\r>");
					getsU1(command,128);
					if (command[0] == 'a' || command[0] == 'A') {
						ALCFGRPTbits.AMASK = 0x4;
						sprintf(toPrint,"OK, set to sample once every 10 minutes.\r");
					} else if (command[0] == 'b' || command[0] == 'B'){
						ALCFGRPTbits.AMASK = 0x5;
						sprintf(toPrint,"OK, set to sample once per hour.\r");
					} else if (command[0] == 'c' || command[0] == 'C'){
						ALCFGRPTbits.AMASK = 0x6;
						sprintf(toPrint,"OK, set to sample once per day.\r");
					} else if (command[0] == 'd' || command[0] == 'D'){
						ALCFGRPTbits.AMASK = 0x7;
						sprintf(toPrint,"OK, set to sample once per week.\r");
					} else {
						sprintf(toPrint,"Sorry, did not understand that option.\r");
					}
				} else if (strncmp(command,"spcl",4) == 0) {
					putsU1("This command will CLEAR ALL DATA FROM THE LOGS!!!\r");
					putsU1("ARE YOU SURE YOU WANT TO DO THIS?(y|[n])\r>");
					getsU1(command,128);
					if (command[0] == 'y' || command[0] == 'Y') {
						// Enable SPI1
						PMD1bits.SPI1MD = 0;

						// Buffer allocation
						char headerBuffer[255];
						// Initialize the File system
						if (FSInit()){
							// Define a pointer to the log file
							FSFILE *logFile;

							// A variable to keep track of how many characters were written 
							int charsWritten;

							// Define the name of the log file
							char logFileName[] = "DATALOG.TXT";

							// The mode to open the file in (a = append, w = write/over-write
							char appendArg[] = "w";

							// Open the file
							logFile = FSfopen(logFileName,appendArg);

							// If the file opened OK, write a header
							if (logFile != NULL) {
								// Write to a buffer first
								charsWritten = sprintf(headerBuffer,"Timestamp,Avg Flow Rate(l/s),Flow Total(lx100),Transmitter Temp(Deg C),Battery Cap(%),Power Status,Fault Code\n");
								// Write those to a file
								FSfwrite(headerBuffer,1,charsWritten,logFile);

								// Close the file
								FSfclose(logFile);
							}
						}

						// Disable SPI1
						PMD1bits.SPI1MD = 1;
						sprintf(toPrint,"OK, log file is cleared.\r");
					} else {
						sprintf(toPrint,"Log file NOT cleared.\r");
					}
				} else if (strncmp(command,"gfdt",4) == 0) {
					unsigned char flowMeterClock[6];
					readActualDateAndTime(flowMeterClock);
					sprintf(toPrint,"Flow Meter Date = 20%02u-%02u-%02uT%02u:%02u:%02u \r", flowMeterClock[0],
						flowMeterClock[1],flowMeterClock[2],flowMeterClock[3],flowMeterClock[4],
						flowMeterClock[5]);
				} else if (strncmp(command,"gfcd",4) == 0) {
					unsigned char calDate[6];
					readCalDateAndTime(calDate);
					sprintf(toPrint,"Flow Meter Calibration Date = 20%02u-%02u-%02uT%02u:%02u:%02u \r", calDate[0],
						calDate[1],calDate[2],calDate[3],calDate[4],
						calDate[5]);
				} else if (strncmp(command,"gfcf",4) == 0) {
					sprintf(toPrint,"Calibration Factor = %5.5f \r", 
						readCalibrationFactor());
				} else if (strncmp(command,"gfoh",4) == 0) {
					sprintf(toPrint,"Operating hours since first power up = %lu \r", 
						readOperatingHoursSincePowerUp());
				} else if (strncmp(command,"gfnp",4) == 0) {
					sprintf(toPrint,"Number of power ups since first power up = %u \r", 
						readNumberOfPowerUps());
				} else if (strncmp(command,"gftf",4) == 0) {
					sprintf(toPrint,"Total flow since stats reset = %ld \r", 
						readTotalizer1Integer());
				} else if (strncmp(command,"gftu",4) == 0) {
					unsigned char totalFlowUnitsBuffer[13];
					readTotalFlowUnits(totalFlowUnitsBuffer);
					sprintf(toPrint,"Units for total flow = %s \r", 
						totalFlowUnitsBuffer);
				} else if (strncmp(command,"gfqn",4) == 0) {
					sprintf(toPrint,"Qn (nominal flow) = %5.5f \r", 
						readQn());
				} else if (strncmp(command,"gffl",4) == 0) {
					// Do a priming read
					readFlowRate();
					sprintf(toPrint,"Current flow rate = %5.5f \r", 
						readFlowRate());
				} else if (strncmp(command,"gffr",4) == 0) {
					sprintf(toPrint,"Current flow rate as percent of Qn = %3.2f%% \r", 
						readFlowratePercentValue());
				} else if (strncmp(command,"gfmx",4) == 0) {
					sprintf(toPrint,"Max flow rate recorded = %5.5f \r", 
						readHighestFlowRate());
				} else if (strncmp(command,"gfmd",4) == 0) {
					unsigned char maxFlowDate[6];
					readHighestFlowDateAndTime(maxFlowDate);
					sprintf(toPrint,"Date of maximum recorded flow = 20%02u-%02u-%02uT%02u:%02u:%02u \r", maxFlowDate[0],
						maxFlowDate[1],maxFlowDate[2],maxFlowDate[3],maxFlowDate[4],
						maxFlowDate[5]);
				} else if (strncmp(command,"gfmn",4) == 0) {
					sprintf(toPrint,"Minimum flow rate recorded = %5.5f \r", 
						readLowestFlowRate());
				} else if (strncmp(command,"gfnd",4) == 0) {
					unsigned char minFlowDate[6];
					readLowestFlowDateAndTime(minFlowDate);
					sprintf(toPrint,"Date of minimum recorded flow = 20%02u-%02u-%02uT%02u:%02u:%02u \r", minFlowDate[0],
						minFlowDate[1],minFlowDate[2],minFlowDate[3],minFlowDate[4],
						minFlowDate[5]);
				} else if (strncmp(command,"gfhc",4) == 0) {
					sprintf(toPrint,"Highest consumption in day = %5.5f \r", 
						readHighestDayConsumption());
				} else if (strncmp(command,"gfhd",4) == 0) {
					unsigned char highestConsumptionDate[6];
					readHighestDayConsumptionDateAndTime(highestConsumptionDate);
					sprintf(toPrint,"Date of highest day consumption = 20%02u-%02u-%02uT%02u:%02u:%02u \r", highestConsumptionDate[0],
						highestConsumptionDate[1],highestConsumptionDate[2],highestConsumptionDate[3],highestConsumptionDate[4],
						highestConsumptionDate[5]);
				} else if (strncmp(command,"gffc",4) == 0) {
					sprintf(toPrint,"Flowrate cutoff (as %% of Qn) = %3.2f%% \r", 
						readLowFlowCutoff());
				} else if (strncmp(command,"gffu",4) == 0) {
					unsigned char flowRateUnitsBuffer[13];
					readFlowRateUnits(flowRateUnitsBuffer);
					sprintf(toPrint,"Units for flow rate = %s \r", 
						flowRateUnitsBuffer);
				} else if (strncmp(command,"gfvl",4) == 0) {
					sprintf(toPrint,"Current Velocity = %5.5f \r", 
						readActualVelocity());
				} else if (strncmp(command,"gftp",4) == 0) {
					sprintf(toPrint,"Current transmitter temp = %3.2f degrees C.\r", 
						readTransmitterTemp());
				} else if (strncmp(command,"gfbt",4) == 0) {
					sprintf(toPrint,"Battery %% of max capacity = %3u%%\r", 
						readBatteryCapacity());
				} else if (strncmp(command,"gfps",4) == 0) {
					sprintf(toPrint,"Power Status = %02u\r", 
						readPowerStatus());
				} else if (strncmp(command,"gfft",4) == 0) {
					sprintf(toPrint,"Fault Status = 0x%o\r", 
						readFaultStatus());
				} else if (strncmp(command,"gfmt",4) == 0) {
					sprintf(toPrint,"Comm module type = %02u\r", 
						readCommModuleType());
				} else if (strncmp(command,"gfll",4) == 0) {
					unsigned char latestLogDate[6];
					readLastLogDate(latestLogDate);
					sprintf(toPrint,"Date of last log entry = 20%02u-%02u-%02uT%02u:%02u:%02u \r", latestLogDate[0],
						latestLogDate[1],latestLogDate[2],latestLogDate[3],latestLogDate[4],
						latestLogDate[5]);
				} else if (strncmp(command,"fsyn",4) == 0) {
					putsU1("Flow Meter Clock will be set to time on PIC\r");
					unsigned char timeSnapshot[6] = {getYear(),getMonth(),getDay(),getHour(),getMin(),getSec()};
					setActualDateAndTime(timeSnapshot);
					putsU1("OK, time set on flow meter.\r");					
				} else {
					if (command[0] == '\0') {
					} else {
						sprintf(toPrint,"Sorry, didn't understand %s \r",command);
					}
				}
				putsU1(toPrint);
			}
		} else {
			// Read the log sample
			readAndLogSample();

			// If the terminal is not active, shut everything down and wait for next interrupt
			if (terminalActive <= 0) {
				// Before it goes to sleep, make sure we reset the
				// bit to enable the UART for the terminal to wake it up
				U1MODE = 0x8288;

				// Put PIC in idle mode
				Idle();
			}
		}
	}

} // end main

