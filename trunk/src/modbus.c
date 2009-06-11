// Functions for modbus interactions
#include "modbus.h"

#include "UART.h"

#include <string.h>
#include <float.h>
#include <stdio.h>

// The buffer that will be used to issue and parse modbus communications
unsigned char buffer[MODBUS_SIZE];	
char messageBuffer[255];

// Function to write the CRC16 check to the data buffer
int CRC16(unsigned int dataLength, char check)
{
	unsigned int CheckSum;
	unsigned int j;
	unsigned char lowCRC;
	unsigned char highCRC;
	unsigned short i;
	CheckSum = 0xffff;
	for (j=0; j<dataLength; j++)
	{
		CheckSum = CheckSum^(unsigned int)buffer[j];
		for(i=8;i>0;i--)
			if((CheckSum)&0x0001)
				CheckSum = (CheckSum>>1)^0xa001;
			else
				CheckSum>>=1;
	}	
	highCRC = CheckSum>>8;
	CheckSum<<=8;
	lowCRC = CheckSum>>8;
	if (check==1)
	{	
		if ( (buffer[dataLength+1] == highCRC) & (buffer[dataLength] == lowCRC ))	
  			return 1;
		else
  			return 0;
	}
	else
	{  
		buffer[dataLength] = lowCRC;
		buffer[dataLength+1] = highCRC;
		return 1; 	
	}
}

// function to send modbus command from buffer
void sendModbusCommand(unsigned char command, 
	unsigned int commandLength){

	// Loop over the length of the command and send those bytes over
	// UART 2
	int i = 0;
	for (i=0; i < commandLength; i++) {
		putU2(buffer[i]);
	}

	// Now wait until something arrives or until we time out 
	// waiting for a response
	unsigned long timeoutCounter = 0;
	while (!charArrivedAtUART2() && (timeoutCounter < 20000)) {
		timeoutCounter++;
	}

	// Now read back the rest of the expected response
	int j;
	for (j = 0; j < MODBUS_SIZE; j++) {
		buffer[j] = getU2();
	}
}

void sendUnlockPassword(void) {
	// The password to send
	unsigned char password[] = "1000";

	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to write to a register 
	// (function code = 10)
	buffer[1] = 0x10;
	// The Address for the holding register for 
	// the password (2008 -> 0x07D8)
	buffer[2] = 0x07;
	buffer[3] = 0xD8;
	// We are going to write 3 registers (6 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x03;
	// The number of bytes
	buffer[6] = 0x06;
	// values
	buffer[7] = password[0];
	buffer[8] = password[1];
	buffer[9] = password[2];
	buffer[10] = password[3];
	buffer[11] = 0x00;
	buffer[12] = 0x00;
	// Now tack on the CRC
	CRC16(13,0);
	// Now send it
	sendModbusCommand(0x10, 15);
}

// The function to read the velocity in mm/s (register 3000)
float readActualVelocity(void) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the actual velocity (3000 -> 0x0BB8)
	buffer[2] = 0x0B;
	buffer[3] = 0xB8;
	// We are going to read 2 registers (4 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x02;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// The four bytes at buffer spots 3, 4, 5 and 6 should
	// be the velocity as a float (but as a float it expects 
	// LSB so we flip the order in the new chars)
	unsigned char velocityChars[4] = {buffer[6],buffer[5],buffer[4],buffer[3]};
	// Convert float and return
	float * velocityPtr = &velocityChars[0];
//	sprintf(messageBuffer,"Current Velocity = %3.5f \r", *velocityPtr);
//	putsU1(messageBuffer);
	return *velocityPtr;
}

// The function to read the flow rate in l/s (register 3002)
float readFlowRate(void) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the flow rate (3002 -> 0x0BBA)
	buffer[2] = 0x0B;
	buffer[3] = 0xBA;
	// We are going to read 2 registers (4 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x02;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// The four bytes at buffer spots 3, 4, 5 and 6 should
	// be the flow as a float (but as a float it expects 
	// LSB so we flip the order in the new chars)
	unsigned char flowChars[4] = {buffer[6],buffer[5],buffer[4],buffer[3]};
	// Convert float and return
	float * flowRatePtr = &flowChars[0];
//	sprintf(messageBuffer,"Current Flow = %3.5f \r", *flowRatePtr);
//	putsU1(messageBuffer);
	return *flowRatePtr;
}

// The function to read the insulation value (register 3004)
float readInsulationValue(void) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the insulation value (3004 -> 0x0BBC)
	buffer[2] = 0x0B;
	buffer[3] = 0xBC;
	// We are going to read 2 registers (4 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x02;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// The four bytes at buffer spots 3, 4, 5 and 6 should
	// be the insulation value as a float (but as a float it expects 
	// LSB so we flip the order in the new chars)
	unsigned char insulationValueChars[4] = {buffer[6],buffer[5],buffer[4],buffer[3]};
	// Convert float and return
	float * insulationValuePtr = &insulationValueChars[0];
//	sprintf(messageBuffer,"Insulation Value = %3.5f \r", *insulationValuePtr);
//	putsU1(messageBuffer);
	return *insulationValuePtr;
}

// The function to read the flowrate percent of Qc (register 3012)
float readFlowratePercentValue(void) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the flowrate percent Qc (3012 -> 0x0BC4)
	buffer[2] = 0x0B;
	buffer[3] = 0xC4;
	// We are going to read 2 registers (4 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x02;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// The four bytes at buffer spots 3, 4, 5 and 6 should
	// be the flowrate percent as a float (but as a float it expects 
	// LSB so we flip the order in the new chars)
	unsigned char flowratePercentChars[4] = {buffer[6],buffer[5],buffer[4],buffer[3]};
	// Convert float and return
	float * flowratePercentPtr = &flowratePercentChars[0];
//	sprintf(messageBuffer,"Flowrate Percent = %3.2f \r", *flowratePercentPtr);
//	putsU1(messageBuffer);
	return *flowratePercentPtr;
}

// The function to read totalizer 1 (register 3017)
void readTotalizer1(void) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// totalizer 1 (3017 -> 0x0BC9)
	buffer[2] = 0x0B;
	buffer[3] = 0xC9;
	// We are going to read 4 registers (8 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x04;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);
}

// The function to return the integer portion of the first totalizer
long readTotalizer1Integer(void) {
	// First read totalizer 1 into buffer
	readTotalizer1();

	// The four bytes at buffer spots 3, 4, 5 and 6 should
	// be the number which is the interger part of totalizer 1
	long totalizer1 = 0;
	totalizer1 |= (buffer[3] << 24);
	totalizer1 |= (buffer[4] << 16);
	totalizer1 |= (buffer[5] << 8);
	totalizer1 |= buffer[6];
//	sprintf(messageBuffer,"Totalizer 1 Integer = %ld \r", totalizer1);
//	putsU1(messageBuffer);
	return totalizer1;
}

// The function to read totalizer 2 (register 3021)
void readTotalizer2(void) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// totalizer 1 (3021 -> 0x0BCD)
	buffer[2] = 0x0B;
	buffer[3] = 0xCD;
	// We are going to read 4 registers (8 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x04;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);
}

// The function to return the integer portion of the second totalizer
long readTotalizer2Integer(void) {
	// First read totalizer 2 into buffer
	readTotalizer2();

	// The four bytes at buffer spots 3, 4, 5 and 6 should
	// be the number which is the interger part of totalizer 2
	long totalizer2 = 0;
	totalizer2 |= (buffer[3] << 24);
	totalizer2 |= (buffer[4] << 16);
	totalizer2 |= (buffer[5] << 8);
	totalizer2 |= buffer[6];
//	sprintf(messageBuffer,"Totalizer 2 Integer = %ld \r", totalizer2);
//	putsU1(messageBuffer);
	return totalizer2;
}

// The function to read the units of total flow (register 216)
void readTotalFlowUnits(unsigned char writeBuffer[]) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the flow rate units is 216 (0x00D8)
	buffer[2] = 0x00;
	buffer[3] = 0xD8;
	// We are going to read 6 registers (12 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x06;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);
	// There should be 12 bytes in the return buffer (3-14)
	writeBuffer[0] = buffer[3];
	writeBuffer[1] = buffer[4];
	writeBuffer[2] = buffer[5];
	writeBuffer[3] = buffer[6];
	writeBuffer[4] = buffer[7];
	writeBuffer[5] = buffer[8];
	writeBuffer[6] = buffer[9];
	writeBuffer[7] = buffer[10];
	writeBuffer[8] = buffer[11];
	writeBuffer[9] = buffer[12];
	writeBuffer[10] = buffer[13];
	writeBuffer[11] = buffer[14];
	writeBuffer[12] = '\0';
}

// The function to read the value for Qn->nominal flow (register 226)
float readQn(void){
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// Qn (226 -> 0x00E2)
	buffer[2] = 0x00;
	buffer[3] = 0xE2;
	// We are going to read 2 registers (4 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x02;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// The four bytes at buffer spots 3, 4, 5 and 6 should
	// be Qn as a float (but as a float it expects 
	// LSB so we flip the order in the new chars)
	unsigned char qnChars[4] = {buffer[6],buffer[5],buffer[4],buffer[3]};
	// Convert float and return
	float * qnPtr = &qnChars[0];
//	sprintf(messageBuffer,"Qn = %3.5f \r", *qnPtr);
//	putsU1(messageBuffer);
	return *qnPtr;
}
// This is the function to read the actual date and time of the
// flow meter.  The unsigned char array that is input will be used
// to record a string of the format yyyy-MM-ddThh:mm:ss.  This
// information is read from register 3033.
void readActualDateAndTime(unsigned char dateAndTimeBuffer[]) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the actual date and time is 3033 = 0x0BD9
	buffer[2] = 0x0B;
	buffer[3] = 0xD9;
	// We are going to read 3 registers (6 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x03;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// Now write the date and time to buffer that came in
	dateAndTimeBuffer[0] = buffer[3];
	dateAndTimeBuffer[1] = buffer[4];
	dateAndTimeBuffer[2] = buffer[5];
	dateAndTimeBuffer[3] = buffer[6];
	dateAndTimeBuffer[4] = buffer[7];
	dateAndTimeBuffer[5] = buffer[8];
//	sprintf(messageBuffer,"Actual Date = 20%02u-%02u-%02uT%02u:%02u:%02u \r", dateAndTimeBuffer[0],
//		dateAndTimeBuffer[1],dateAndTimeBuffer[2],dateAndTimeBuffer[3],dateAndTimeBuffer[4],
//		dateAndTimeBuffer[5]);
//	putsU1(messageBuffer);
}

void setActualDateAndTime(unsigned char dateAndTimeBuffer[]) {
	// First send the password
	sendUnlockPassword();

	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to write multiple registers 
	// (function code = 0x10)
	buffer[1] = 0x10;
	// The Address for the actual date and time starts
	// at register 3033 => 0x0BD9
	buffer[2] = 0x0B;
	buffer[3] = 0xD9;
	// We are going to write 3 registers (6 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x03;
	// The number of bytes to be written (3x2 = 6)
	buffer[6] = 0x06;
	// Now the 6 bytes to write
	buffer[7] = dateAndTimeBuffer[0];
	buffer[8] = dateAndTimeBuffer[1];
	buffer[9] = dateAndTimeBuffer[2];
	buffer[10] = dateAndTimeBuffer[3];
	buffer[11] = dateAndTimeBuffer[4];
	buffer[12] = dateAndTimeBuffer[5];
	// Now tack on the CRC
	CRC16(13,0);
	// Now send it
	sendModbusCommand(0x10, 15);

}

// This is the function to read the last calibration date.  This
// information is read from register 230.
void readCalDateAndTime(unsigned char dateAndTimeBuffer[]) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the calibration date and time is 230 = 0x00E6
	buffer[2] = 0x00;
	buffer[3] = 0xE6;
	// We are going to read 3 registers (6 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x03;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// Now write the date and time to buffer that came in
	dateAndTimeBuffer[0] = buffer[3];
	dateAndTimeBuffer[1] = buffer[4];
	dateAndTimeBuffer[2] = buffer[5];
	dateAndTimeBuffer[3] = buffer[6];
	dateAndTimeBuffer[4] = buffer[7];
	dateAndTimeBuffer[5] = buffer[8];
//	sprintf(messageBuffer,"Calibration Date = 20%02u-%02u-%02uT%02u:%02u:%02u \r", dateAndTimeBuffer[0],
//		dateAndTimeBuffer[1],dateAndTimeBuffer[2],dateAndTimeBuffer[3],dateAndTimeBuffer[4],
//		dateAndTimeBuffer[5]);
//	putsU1(messageBuffer);
}

// The function to read the calibration factor (register 228)
float readCalibrationFactor(void) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the calibration factor (228 -> 0x00E4)
	buffer[2] = 0x00;
	buffer[3] = 0xE4;
	// We are going to read 2 registers (4 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x02;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// The four bytes at buffer spots 3, 4, 5 and 6 should
	// be the calibration factor as a float (but as a float it expects 
	// LSB so we flip the order in the new chars)
	unsigned char calFactorChars[4] = {buffer[6],buffer[5],buffer[4],buffer[3]};
	// Convert float and return
	float * calFactorPtr = &calFactorChars[0];
//	sprintf(messageBuffer,"Calibration Factor = %3.5f \r", *calFactorPtr);
//	putsU1(messageBuffer);
	return *calFactorPtr;
}

// The function to read the number of hours since first power up
// (register 80)
unsigned long readOperatingHoursSincePowerUp(void){
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the operating hours since first power up (80 -> 0x0050)
	buffer[2] = 0x00;
	buffer[3] = 0x50;
	// We are going to read 2 registers (4 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x02;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// The four bytes at buffer spots 3, 4, 5 and 6 should
	// be the hours as unsigned long
	unsigned long opHours = 0;
	opHours |= (buffer[3] << 24);
	opHours |= (buffer[4] << 16);
	opHours |= (buffer[5] << 8);
	opHours |= buffer[6];
	return opHours;
}

// The function to read the number of power ups since first power up
// (register 366)
unsigned int readNumberOfPowerUps(void){
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the number of power ups (366 -> 0x016E)
	buffer[2] = 0x01;
	buffer[3] = 0x6E;
	// We are going to read 1 register
	buffer[4] = 0x00;
	buffer[5] = 0x01;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// The two bytes at buffer spots 3 and 4
	unsigned int numberOfPowerUps;
	numberOfPowerUps = (unsigned int)(((unsigned int)buffer[3] << 8) | buffer[4]);
	return numberOfPowerUps;
}

// The function to read the maximum flow rate seen (register 407)
float readHighestFlowRate(void){
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the highest recorded flow (407 -> 0x0197)
	buffer[2] = 0x01;
	buffer[3] = 0x97;
	// We are going to read 2 registers (4 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x02;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// The four bytes at buffer spots 3, 4, 5 and 6 should
	// be the highest flow as a float (but as a float it expects 
	// LSB so we flip the order in the new chars)
	unsigned char highestFlowChars[4] = {buffer[6],buffer[5],buffer[4],buffer[3]};
	// Convert float and return
	float * highestFlowPtr = &highestFlowChars[0];
//	sprintf(messageBuffer,"Highest Flow = %3.5f \r", *highestFlowPtr);
//	putsU1(messageBuffer);
	return *highestFlowPtr;
}
// This is the function to read the date when the highest flow was recorded.  This
// information is read from register 409.
void readHighestFlowDateAndTime(unsigned char dateAndTimeBuffer[]) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the highest flow date and time is 409 = 0x0199
	buffer[2] = 0x01;
	buffer[3] = 0x99;
	// We are going to read 3 registers (6 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x03;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// Now write the date and time to buffer that came in
	dateAndTimeBuffer[0] = buffer[3];
	dateAndTimeBuffer[1] = buffer[4];
	dateAndTimeBuffer[2] = buffer[5];
	dateAndTimeBuffer[3] = buffer[6];
	dateAndTimeBuffer[4] = buffer[7];
	dateAndTimeBuffer[5] = buffer[8];
//	sprintf(messageBuffer,"Highest Flow Date = 20%02u-%02u-%02uT%02u:%02u:%02u \r", dateAndTimeBuffer[0],
//		dateAndTimeBuffer[1],dateAndTimeBuffer[2],dateAndTimeBuffer[3],dateAndTimeBuffer[4],
//		dateAndTimeBuffer[5]);
//	putsU1(messageBuffer);
}

// The function to read the minimum flow rate seen (register 412)
float readLowestFlowRate(void){
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the lowest recorded flow (412 -> 0x019C)
	buffer[2] = 0x01;
	buffer[3] = 0x9C;
	// We are going to read 2 registers (4 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x02;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// The four bytes at buffer spots 3, 4, 5 and 6 should
	// be the lowest flow as a float (but as a float it expects 
	// LSB so we flip the order in the new chars)
	unsigned char lowestFlowChars[4] = {buffer[6],buffer[5],buffer[4],buffer[3]};
	// Convert float and return
	float * lowestFlowPtr = &lowestFlowChars[0];
//	sprintf(messageBuffer,"Lowest Flow = %3.5f \r", *lowestFlowPtr);
//	putsU1(messageBuffer);
	return *lowestFlowPtr;
}
// This is the function to read the date when the lowest flow was recorded.  This
// information is read from register 414.
void readLowestFlowDateAndTime(unsigned char dateAndTimeBuffer[]) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the lowest flow date and time is 409 = 0x019E
	buffer[2] = 0x01;
	buffer[3] = 0x9E;
	// We are going to read 3 registers (6 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x03;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// Now write the date and time to buffer that came in
	dateAndTimeBuffer[0] = buffer[3];
	dateAndTimeBuffer[1] = buffer[4];
	dateAndTimeBuffer[2] = buffer[5];
	dateAndTimeBuffer[3] = buffer[6];
	dateAndTimeBuffer[4] = buffer[7];
	dateAndTimeBuffer[5] = buffer[8];
//	sprintf(messageBuffer,"Lowest Flow Date = 20%02u-%02u-%02uT%02u:%02u:%02u \r", dateAndTimeBuffer[0],
//		dateAndTimeBuffer[1],dateAndTimeBuffer[2],dateAndTimeBuffer[3],dateAndTimeBuffer[4],
//		dateAndTimeBuffer[5]);
//	putsU1(messageBuffer);
}

// The function to read the highest consumption seen in a day (register 417)
float readHighestDayConsumption(void){
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the highest day consumption (417 -> 0x01A1)
	buffer[2] = 0x01;
	buffer[3] = 0xA1;
	// We are going to read 2 registers (4 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x02;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// The four bytes at buffer spots 3, 4, 5 and 6 should
	// be the highest day consumption as a float (but as a float it expects 
	// LSB so we flip the order in the new chars)
	unsigned char highestDayConsumptionChars[4] = {buffer[6],buffer[5],buffer[4],buffer[3]};
	// Convert float and return
	float * highestDayConsumptionPtr = &highestDayConsumptionChars[0];
//	sprintf(messageBuffer,"Highest day consumption = %3.5f \r", *highestDayConsumptionPtr);
//	putsU1(messageBuffer);
	return *highestDayConsumptionPtr;
}
// This is the function to read the date when the highest consumption was recorded.  This
// information is read from register 419.
void readHighestDayConsumptionDateAndTime(unsigned char dateAndTimeBuffer[]) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the highest daily consumption date and time is 419 = 0x01A3
	buffer[2] = 0x01;
	buffer[3] = 0xA3;
	// We are going to read 3 registers (6 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x03;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// Now write the date and time to buffer that came in
	dateAndTimeBuffer[0] = buffer[3];
	dateAndTimeBuffer[1] = buffer[4];
	dateAndTimeBuffer[2] = buffer[5];
	dateAndTimeBuffer[3] = buffer[6];
	dateAndTimeBuffer[4] = buffer[7];
	dateAndTimeBuffer[5] = buffer[8];
//	sprintf(messageBuffer,"Date of highest consumption = 20%02u-%02u-%02uT%02u:%02u:%02u \r", dateAndTimeBuffer[0],
//		dateAndTimeBuffer[1],dateAndTimeBuffer[2],dateAndTimeBuffer[3],dateAndTimeBuffer[4],
//		dateAndTimeBuffer[5]);
//	putsU1(messageBuffer);
}

// The function to read the cutoff flow rate which is the rate
// that anything less will be reported as 0 (register 239)
float readLowFlowCutoff(void){
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the cutoff flow rate (239 -> 0x00EF)
	buffer[2] = 0x00;
	buffer[3] = 0xEF;
	// We are going to read 2 registers (4 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x02;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// The four bytes at buffer spots 3, 4, 5 and 6 should
	// be the cutoff flow rate as a float (but as a float it expects 
	// LSB so we flip the order in the new chars)
	unsigned char cutoffFlowrateChars[4] = {buffer[6],buffer[5],buffer[4],buffer[3]};
	// Convert float and return
	float * cutoffFlowratePtr = &cutoffFlowrateChars[0];
//	sprintf(messageBuffer,"Flowrate cut-off = %3.5f \r", *cutoffFlowratePtr);
//	putsU1(messageBuffer);
	return *cutoffFlowratePtr;
}

// The function to read the units of flow rate (register 210)
void readFlowRateUnits(unsigned char writeBuffer[]) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the flow rate units is 210 (0x00D2)
	buffer[2] = 0x00;
	buffer[3] = 0xD2;
	// We are going to read 6 registers (12 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x06;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);
	// There should be 12 bytes in the return buffer (3-14)
	writeBuffer[0] = buffer[3];
	writeBuffer[1] = buffer[4];
	writeBuffer[2] = buffer[5];
	writeBuffer[3] = buffer[6];
	writeBuffer[4] = buffer[7];
	writeBuffer[5] = buffer[8];
	writeBuffer[6] = buffer[9];
	writeBuffer[7] = buffer[10];
	writeBuffer[8] = buffer[11];
	writeBuffer[9] = buffer[12];
	writeBuffer[10] = buffer[13];
	writeBuffer[11] = buffer[14];
	writeBuffer[12] = '\0';
}

// The function to read the temperature of the transmitter (register 3042)
float readTransmitterTemp(void){
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the transmitter temp (3042 -> 0x0BE2)
	buffer[2] = 0x0B;
	buffer[3] = 0xE2;
	// We are going to read 2 registers (4 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x02;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// The four bytes at buffer spots 3, 4, 5 and 6 should
	// be the transmitter temp as a float (but as a float it expects 
	// LSB so we flip the order in the new chars)
	unsigned char transmitterTempChars[4] = {buffer[6],buffer[5],buffer[4],buffer[3]};
	// Convert float and return
	float * transmitterTempPtr = &transmitterTempChars[0];
//	sprintf(messageBuffer,"Transmitter Temp = %3.5f \r", *transmitterTempPtr);
//	putsU1(messageBuffer);
	return *transmitterTempPtr;
}

// The function to read the actualy battery capacity (register 3030)
unsigned char readBatteryCapacity(void) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the battery capacity (3030 -> 0x0BD6)
	buffer[2] = 0x0B;
	buffer[3] = 0xD6;
	// We are going to read 1 register (2 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x01;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);
	
	// Now buffer spots 3 and 4 contain the reply, but since we are only
	// interested in 4, return that
	unsigned char batteryCapacity = buffer[4];
	return batteryCapacity;
}

// The function to read the power status (register 3031)
unsigned char readPowerStatus(void) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the power status (3031 -> 0x0BD7)
	buffer[2] = 0x0B;
	buffer[3] = 0xD7;
	// We are going to read 1 register (2 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x01;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);
	
	// Now buffer spots 3 and 4 contain the reply, but since we are only
	// interested in 4, return that
	unsigned char powerStatus = buffer[4];
	return powerStatus;
}

// The function to read the fault status (register 3016)
unsigned int readFaultStatus(void) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the fault status is 3016 (0x0BC8)
	buffer[2] = 0x0B;
	buffer[3] = 0xC8;
	// We are going to read 1
	buffer[4] = 0x00;
	buffer[5] = 0x01;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);
	// The two bytes at buffer spots 3 and 4
	unsigned int faultStatus;
	faultStatus = (unsigned int)(((unsigned int)buffer[3] << 8) | buffer[4]);
	return faultStatus;
}

// The function to read the communication module type (register 822)
unsigned char readCommModuleType(void){
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the comm module type (822 -> 0x0336)
	buffer[2] = 0x03;
	buffer[3] = 0x36;
	// We are going to read 1 register (2 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x01;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);
	
	// Now buffer spots 3 and 4 contain the reply, but since we are only
	// interested in 4, return that
	unsigned char commModuleType = buffer[4];
	return commModuleType;
}

// The function to read the date of the last data log entry (register 476)
void readLastLogDate(unsigned char dateAndTimeBuffer[]){
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the date of last log entry 476 = 0x01DC
	buffer[2] = 0x01;
	buffer[3] = 0xDC;
	// We are going to read 3 registers (6 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x03;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// Now write the date and time to buffer that came in
	dateAndTimeBuffer[0] = buffer[3];
	dateAndTimeBuffer[1] = buffer[4];
	dateAndTimeBuffer[2] = buffer[5];
	dateAndTimeBuffer[3] = buffer[6];
	dateAndTimeBuffer[4] = buffer[7];
	dateAndTimeBuffer[5] = buffer[8];
//	sprintf(messageBuffer,"Date of last log entry = 20%02u-%02u-%02uT%02u:%02u:%02u \r", dateAndTimeBuffer[0],
//		dateAndTimeBuffer[1],dateAndTimeBuffer[2],dateAndTimeBuffer[3],dateAndTimeBuffer[4],
//		dateAndTimeBuffer[5]);
//	putsU1(messageBuffer);

}
// --------------------------------------------------------------------------
// NOTE: These method have NOT been tested
// --------------------------------------------------------------------------

// The function to return the fractional portion of the first totalizer
long readTotalizer1Fraction(void) {
	// First read totalizer 1 into buffer
	readTotalizer1();

	// The four bytes at buffer spots 7, 8, 9 and 10 should
	// be the number which is the fractional part of totalizer 1
	long totalizer1Fraction = 0;
	totalizer1Fraction |= (buffer[7] << 24);
	totalizer1Fraction |= (buffer[8] << 16);
	totalizer1Fraction |= (buffer[9] << 8);
	totalizer1Fraction |= buffer[10];
//	sprintf(messageBuffer,"Totalizer 1 Fraction = %ld \r", totalizer1Fraction);
//	putsU1(messageBuffer);
	return totalizer1Fraction;
}

// The function to return the fractional portion of the second totalizer
long readTotalizer2Fraction(void) {
	// First read totalizer 2 into buffer
	readTotalizer2();

	// The four bytes at buffer spots 7, 8, 9 and 10 should
	// be the number which is the fractional part of totalizer 2
	long totalizer2Fraction = 0;
	totalizer2Fraction |= (buffer[7] << 24);
	totalizer2Fraction |= (buffer[8] << 16);
	totalizer2Fraction |= (buffer[9] << 8);
	totalizer2Fraction |= buffer[10];
//	sprintf(messageBuffer,"Totalizer 2 Fraction = %ld \r", totalizer2Fraction);
//	putsU1(messageBuffer);
	return totalizer2Fraction;
}

// The method to retrieve the product ID (should be 10779 for MAG 8000) (register 79)
unsigned int readProductID(void) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the the product ID (79 -> 0x004F)
	buffer[2] = 0x00;
	buffer[3] = 0x4F;
	// We are going to read 1 register
	buffer[4] = 0x00;
	buffer[5] = 0x01;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// The two bytes at buffer spots 3 and 4
	unsigned int productID;
	productID = (unsigned int)(((unsigned int)buffer[3] << 8) | buffer[4]);
	return productID;
}

// Read the device address (register 528)
unsigned int readDeviceAddress(void) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the device address is 528 (0x0210)
	buffer[2] = 0x02;
	buffer[3] = 0x10;
	// We are going to read 1
	buffer[4] = 0x00;
	buffer[5] = 0x01;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);
	// The two bytes at buffer spots 3 and 4
	unsigned int deviceAddress;
	deviceAddress = (unsigned int)(((unsigned int)buffer[3] << 8) | buffer[4]);
	return deviceAddress;
}

// The method to retrieve the number of parity errors (address 500)
unsigned int getNumberOfParityErrors() {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the number of parity errors (500 -> 0x01F4)
	buffer[2] = 0x01;
	buffer[3] = 0xF4;
	// We are going to read 1
	buffer[4] = 0x00;
	buffer[5] = 0x01;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// The two bytes at buffer spots 3 and 4
	unsigned int numberParityErrors;
	numberParityErrors = (unsigned int)(((unsigned int)buffer[3] << 8) | buffer[4]);
	return numberParityErrors;
}

// The method to retrieve the baud rate as an unsigned long (address 514)
unsigned long getBaudRateAsUnsignedLong() {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the the baud rate as unsigned long (514 -> 0x0202)
	buffer[2] = 0x02;
	buffer[3] = 0x02;
	// We are going to read 2 registers (4 bytes)
	buffer[4] = 0x00;
	buffer[5] = 0x02;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// The four bytes at buffer spots 3, 4, 5 and 6 should
	// be the baud rate as unsigned long
	unsigned long brul = 0;
	brul |= (buffer[3] << 24);
	brul |= (buffer[4] << 16);
	brul |= (buffer[5] << 8);
	brul |= buffer[6];
	return brul;
}

long getBaudRate(void) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the baud rate is 529 = 0x0211.  Remember we
	// write them MSB first
	buffer[2] = 0x02;
	buffer[3] = 0x0E;
	// We are going to only read one register but
	// again we specify as two bytes in MSB
	buffer[4] = 0x00;
	buffer[5] = 0x010;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// OK, assuming no error, the two bytes at positions
	// 3 and 4 should contain the hex value for the baud rate
	if (buffer[3] == 0x00) {
		if (buffer[4] == 0x00) return 1200l;
		if (buffer[4] == 0x01) return 2400l;
		if (buffer[4] == 0x02) return 4800l;
		if (buffer[4] == 0x03) return 9600l;
		if (buffer[4] == 0x04) return 19200l;
		if (buffer[4] == 0x05) return 38400l;
		if (buffer[4] == 0x06) return 57600l;
		if (buffer[4] == 0x07) return 76800l;
		if (buffer[4] == 0x08) return 115200l;
	}
	// Return 0 if nothing matches
	return 0l;
}
// This is the method to retreive the name of the manufacturer
void getManufacturerName(void) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the manufacturer name is 607-612 (0x025F + 5)
	buffer[2] = 0x02;
	buffer[3] = 0x5F;
	// We are going to read 6
	buffer[4] = 0x00;
	buffer[5] = 0x06;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);
//	putsU1("getManufacturerName:");
//	putsU1(buffer);
}

// This is the function to read the temperature from
// the flow sensor
float * readSensorTemperature(void) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// temperature is 3006-3307 = 0x0BBE.  Remember we
	// write them MSB first
	buffer[2] = 0x0B;
	buffer[3] = 0xBE;
	// We are going to only read two registers but
	// again we specify as two bytes in MSB
	buffer[4] = 0x00;
	buffer[5] = 0x02;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// OK, assuming no error, let's look at positions
	// 3, 4, 5 and 6 of the buffer which should contain the
	// MSB and LSB of the temperature in C
	unsigned char tempChars[4] = {buffer[3],buffer[4],buffer[5],buffer[6]};

	// Now return the address of the
	return &tempChars;
}


// Report back running status
int isRunning(void) {
	// First construct the request
	// Slave ID
	buffer[0] = 0x01;
	// The function code to read a holding register 
	// (function code = 3)
	buffer[1] = 0x03;
	// The Address for the holding register for 
	// the running status is 601 = 0x0259.  Remember we
	// write them MSB first
	buffer[2] = 0x02;
	buffer[3] = 0x59;
	// We are going to only read one register but
	// again we specify as two bytes in MSB
	buffer[4] = 0x00;
	buffer[5] = 0x01;
	// Now tack on the CRC
	CRC16(6,0);
	// Now send it
	sendModbusCommand(0x03, 8);

	// OK, assuming no error, the two bytes at positions
	// 3 and 4 should contain the hex value for the running status
	if (buffer[3] == 0x00) {
		if (buffer[4] == 0x00) return 0;
		if (buffer[4] == 0xFF) return 1;
	}
	// Return 0 if nothing matches
	return 2;
}


// Call the method to report the slave ID
unsigned char * reportSlaveID(void) {
	// This is the length of the request
	int requestLength = 4;

	// The first thing to do is take the incoming buffer and
	// fill it with the command structure that is needed to
	// ask for the slave ID

	// Slave address = 1
	buffer[0] = 0x01;

	// Function ID = 17
	buffer[1] = 0x11;

	// Tack on the two byte CRC
	CRC16(2,0);

	// Now send the command
	sendModbusCommand(0x11, requestLength);

	// Now return the buffer pointer
	return buffer;
}
