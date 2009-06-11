/**********************************************************************
* ?2007 Microchip Technology Inc.
*
* FileName:        RTCC.c
* Dependencies:    Header (.h) files if applicable, see below
* Processor:       PIC24Fxxxx
* Compiler:        MPLAB?C30 v3.00 or higher
*
* SOFTWARE LICENSE AGREEMENT:
* Microchip Technology Incorporated ("Microchip") retains all 
* ownership and intellectual property rights in the code accompanying
* this message and in all derivatives hereto.  You may use this code,
* and any derivatives created by any person or entity by or on your 
* behalf, exclusively with Microchip's proprietary products.  Your 
* acceptance and/or use of this code constitutes agreement to the 
* terms and conditions of this notice.
*
* CODE ACCOMPANYING THIS MESSAGE IS SUPPLIED BY MICROCHIP "AS IS". NO 
* WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT 
* NOT LIMITED TO, IMPLIED WARRANTIES OF NON-INFRINGEMENT, 
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS 
* CODE, ITS INTERACTION WITH MICROCHIP'S PRODUCTS, COMBINATION WITH 
* ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
*
* YOU ACKNOWLEDGE AND AGREE THAT, IN NO EVENT, SHALL MICROCHIP BE 
* LIABLE, WHETHER IN CONTRACT, WARRANTY, TORT (INCLUDING NEGLIGENCE OR
* BREACH OF STATUTORY DUTY), STRICT LIABILITY, INDEMNITY, 
* CONTRIBUTION, OR OTHERWISE, FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
* EXEMPLARY, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, FOR COST OR 
* EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE CODE, HOWSOEVER 
* CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE
* DAMAGES ARE FORESEEABLE.  TO THE FULLEST EXTENT ALLOWABLE BY LAW, 
* MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN ANY WAY RELATED TO THIS
* CODE, SHALL NOT EXCEED THE PRICE YOU PAID DIRECTLY TO MICROCHIP 
* SPECIFICALLY TO HAVE THIS CODE DEVELOPED.
*
* You agree that you are solely responsible for testing the code and 
* determining its suitability.  Microchip has no obligation to modify,
* test, certify, or support the code.
*
* REVISION HISTORY:
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Author       	Date      	Comments on this revision
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
* Albert Z.		03/24/08	First release of source file
*
*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*
* ADDITIONAL NOTES:
* Copy the RTCC functions to your application. 
*
* Code Tested on:
* Explorer 16 Demo board with  PIC24FJ128GA010 controller
*
* DESCRIPTION:
* Microchip's PIC24F microcontrollers feature a useful Real Time Clock
* for users to get an on chip clock in their applications. 
* This code example demonstrates how to grab and set the RTC. 
**********************************************************************/
#include <p24fxxxx.h>

// Union to access rtcc registers
typedef union tagRTCC {
	struct {
		unsigned char sec;
		unsigned char min;
		unsigned char hr;
		unsigned char wkd;
		unsigned char day;
		unsigned char mth;
		unsigned char yr;
	};
	struct {
		unsigned int prt00;
		unsigned int prt01;
		unsigned int prt10;
		unsigned int prt11;
	};
} RTCC;

RTCC _time;
RTCC _time_chk;
RTCC _alarm;

// macro
#define mRTCCDec2Bin(Dec) (10*(Dec>>4)+(Dec&0x0f))
#define mRTCCBin2Dec(Bin) (((Bin/10)<<4)|(Bin%10))

unsigned char getHour(void);
unsigned char getMin(void);
unsigned char getSec(void);
unsigned char getMonth(void);
unsigned char getDay(void);
unsigned char getYear(void);

void RTCCgrab(void);
void RTCCInit(void);
void RTCCSet(void);
void RTCCAlarmSet(void);
void RTCCUnlock(void);
void RTCCSetBinSec(unsigned char, int);
void RTCCSetBinMin(unsigned char, int );
void RTCCSetBinHour(unsigned char, int );
void RTCCCalculateWeekDay(void);
void RTCCSetBinDay(unsigned char, int );
void RTCCSetBinMonth(unsigned char, int );
void RTCCSetBinYear(unsigned char, int );

unsigned char getHour(void) {
	return mRTCCDec2Bin(_time_chk.hr);
}
unsigned char getMin(void) {
	return mRTCCDec2Bin(_time_chk.min);
}
unsigned char getSec(void) {
	return mRTCCDec2Bin(_time_chk.sec);
}
unsigned char getMonth(void) {
	return mRTCCDec2Bin(_time_chk.mth);
}
unsigned char getDay(void) {
	return mRTCCDec2Bin(_time_chk.day);
}
unsigned char getYear(void) {
	return mRTCCDec2Bin(_time_chk.yr);
}
/*********************************************************************
 * Function: RTCCProcessEvents
 *
 * Preconditions: RTCCInit must be called before.
 *
 * Overview: The function grabs the current time from the RTCC and
 * translate it into strings.
 *
 * Input: None.
 *
 * Output: It update time and date strings  _time_str, _date_str,
 * and _time, _time_chk structures.
 *
 ********************************************************************/
void RTCCgrab(void)
{
	// Process time object only if time is not being set

		// Grab the time
		RCFGCALbits.RTCPTR = 0;			
		_time.prt00 = RTCVAL;
		RCFGCALbits.RTCPTR = 1;			
		_time.prt01 = RTCVAL;
		RCFGCALbits.RTCPTR = 2;			
		_time.prt10 = RTCVAL;
		RCFGCALbits.RTCPTR = 3;			
		_time.prt11 = RTCVAL;

		// Grab the time again 
		RCFGCALbits.RTCPTR = 0;			
		_time_chk.prt00 = RTCVAL;
		RCFGCALbits.RTCPTR = 1;			
		_time_chk.prt01 = RTCVAL;
		RCFGCALbits.RTCPTR = 2;			
		_time_chk.prt10 = RTCVAL;
		RCFGCALbits.RTCPTR = 3;			
		_time_chk.prt11 = RTCVAL;

		// Verify there is no roll-over
		if ((_time.prt00 == _time_chk.prt00) &&
			(_time.prt01 == _time_chk.prt01) &&
			(_time.prt10 == _time_chk.prt10) &&
			(_time.prt11 == _time_chk.prt11))
		{
			// Here, you can watch structure _time, 
			//	which has the data from RTCC registers.
 
			// TO DO: do something as you like here. 
		}
	
}

/*********************************************************************
 * Function: RTCCInit
 *
 * Preconditions: RTCCInit must be called before.
 *
 * Overview: Enable the oscillator for the RTCC
 *
 * Input: None.
 *
 * Output: None.
 ********************************************************************/
void RTCCInit(void)
{
    // Enables the LP OSC for RTCC operation
	asm("mov #OSCCON,W1");	// move address of OSCCON to W1
	asm("mov.b #0x02, W0");	// move 8-bit literal to W0, 16-bit.
	asm("mov.b #0x46, W2");	// unlock byte 1 for OSCCONL(low byte)
	asm("mov.b #0x57, W3");	// unlock byte 2 for OSCCONL(low byte)
							// move 8-bit of Wn to OSCCON register
	asm("mov.b W2, [W1]");	// write unlock byte 1
	asm("mov.b W3, [W1]");	// write unlock byte 2
	asm("mov.b W0, [W1]");	// enable SOSCEN

    // Unlock sequence must take place for RTCEN to be written
	RCFGCAL	= 0x0000;
    RTCCUnlock();
    RCFGCALbits.RTCEN = 1;	// bit15
//    RCFGCALbits.RTCOE = 1;	// bit10 enable output of alarm from rtcc
    
    // TO DO: Write the time and date to RTCC as follow. 
	_time_chk.sec = 0x00;
	_time_chk.min = 0x58;
	_time_chk.hr = 0x23;
	_time_chk.wkd = 0x2; 
	_time_chk.day = 0x31;
	_time_chk.mth = 0x12;
	_time_chk.yr = 0x08;
	RTCCCalculateWeekDay();	// To calculate and confirm the weekday

	// Set it after you change the time and date. 
	RTCCSet();
}

/*********************************************************************
 * Function: RTCCSet
 *
 * Preconditions: None.
 *
 * Overview: 
 * The function upload time and date from _time_chk into clock.
 *
 * Input: _time_chk - structure containing time and date.
 *
 * Output: None.
 *
 ********************************************************************/
void RTCCSet(void)
{
	RTCCUnlock();				// Unlock the RTCC
	// Set the time
	RCFGCALbits.RTCPTR = 0;		// set min:sec
	RTCVAL = _time_chk.prt00;
	RCFGCALbits.RTCPTR = 1;		// set week:hour
	RTCVAL = _time_chk.prt01;
	RCFGCALbits.RTCPTR = 2;		// set month:day
	RTCVAL = _time_chk.prt10;
	RCFGCALbits.RTCPTR = 3;		// set year
	RTCVAL = _time_chk.prt11;
	RCFGCALbits.RTCWREN = 0;	// Lock the RTCC
	
	// Here, you can watch the RTCC register, 
	// 	the new time and date has been updated. 
}

void RTCCAlarmSet(void)
{
	ALCFGRPTbits.ALRMEN = 0;				// Disable the alarm
	// Set the time
	ALCFGRPTbits.ALRMPTR = 0;		// set min:sec
	ALRMVAL = _alarm.prt00;
	ALCFGRPTbits.ALRMPTR = 1;		// set week:hour
	RTCVAL = _alarm.prt01;
	ALCFGRPTbits.ALRMPTR = 2;		// set month:day
	RTCVAL = _alarm.prt10;
	ALCFGRPTbits.ALRMPTR = 3;		// set year
	RTCVAL = _alarm.prt11;
}

/*********************************************************************
 * Function: RTCCUnlock
 *
 * Preconditions: None.
 *
 * Overview: The function allows a writing into the clock registers.
 *
 * Input: None.
 *
 * Output: None.
 *
 ********************************************************************/
void RTCCUnlock()
{
	asm volatile("disi	#5");
	asm volatile("mov #0x55, w7");		// write 0x55 and 0xAA to
	asm volatile("mov w7, _NVMKEY"); 	//  NVMKEY to disable
	asm volatile("mov #0xAA, w8");		// 	write protection
	asm volatile("mov w8, _NVMKEY");
    asm volatile("bset _RCFGCAL, #13");	// set the RTCWREN bit
	asm volatile("nop");
	asm volatile("nop");
}

/*********************************************************************
 * Function: RTCCSetBinSec
 *
 * Preconditions: None.
 *
 * Overview: The function verifies setting seconds range, translates
 * it into BCD format and writes into _time_chk structure. To write
 * the structure into clock RTCCSet must be called.
 *
 * Input: Seconds binary value.
 *        int to indicate if this is for the alarm (1) or not(0)
 *
 * Output: Checked BCD value in _time_chk structure.
 *
 ********************************************************************/
void RTCCSetBinSec(unsigned char Sec, int forAlarm)
{
    if(Sec >= 60)  Sec = 0;
	if (forAlarm > 0) {
	    _alarm.sec = mRTCCBin2Dec(Sec);
	} else {
	    _time_chk.sec = mRTCCBin2Dec(Sec);
	}
}

/*********************************************************************
 * Function: RTCCSetBinMin
 *
 * Preconditions: None.
 *
 * Overview: The function verifies a setting minutes range, translates
 * it into BCD format and writes into _time_chk structure. To write
 * the structure into clock RTCCSet must be called.
 *
 * Input: Minutes binary value.
 *        int to indicate if this is for the alarm (1) or not(0)
 *
 * Output: Checked BCD value in _time_chk structure.
 *
 ********************************************************************/
void RTCCSetBinMin(unsigned char Min, int forAlarm)
{
    if(Min >= 60)  Min = 0;
	if (forAlarm > 0) {
	    _alarm.min = mRTCCBin2Dec(Min);
	} else {
 	   _time_chk.min = mRTCCBin2Dec(Min);
	}
}

/*********************************************************************
 * Function: RTCCSetBinHour
 *
 * Preconditions: None.
 *
 * Overview: The function verifies a setting hours range, translates
 * it into BCD format and writes into _time_chk structure. To write
 * the structure into clock RTCCSet must be called.
 *
 * Input: Hours binary value.
 *        int to indicate if this is for the alarm (1) or not(0)
 *
 * Output: Checked BCD value in _time_chk structure.
 *
 ********************************************************************/
void RTCCSetBinHour(unsigned char Hour, int forAlarm)
{
    if(Hour >= 24) Hour = 0;
	if (forAlarm > 0) {
	    _alarm.hr = mRTCCBin2Dec(Hour);
	} else {
	    _time_chk.hr = mRTCCBin2Dec(Hour);
	}
}

/*********************************************************************
 * Function: RTCCCalculateWeekDay
 *
 * Preconditions: 
 * Valid values of day, month and year must be presented in 
 * _time_chk structure.
 *
 * Overview: The function reads day, month and year from _time_chk and 
 * calculates week day. Than It writes result into _time_chk. To write
 * the structure into clock RTCCSet must be called.
 *
 * Input: _time_chk with valid values of day, month and year.
 *
 * Output: Zero based week day in _time_chk structure.
 *
 ********************************************************************/
void RTCCCalculateWeekDay()
{
	const char MonthOffset[] =
	//jan feb mar apr may jun jul aug sep oct nov dec
	{   0,  3,  3,  6,  1,  4,  6,  2,  5,  0,  3,  5 };
	unsigned Year;
	unsigned Month;
	unsigned Day;
	unsigned Offset;
    // calculate week day 
    Year  = mRTCCDec2Bin(_time_chk.yr);
    Month = mRTCCDec2Bin(_time_chk.mth);
    Day  = mRTCCDec2Bin(_time_chk.day);
    
    // 2000s century offset = 6 +
    // every year 365%7 = 1 day shift +
    // every leap year adds 1 day
    Offset = 6 + Year + Year/4;
    // Add month offset from table
    Offset += MonthOffset[Month-1];
    // Add day
    Offset += Day;

    // If it's a leap year and before March there's no additional day yet
    if((Year%4) == 0)
        if(Month < 3)
            Offset -= 1;
    
    // Week day is
    Offset %= 7;

    _time_chk.wkd = Offset;
}

/*********************************************************************
 * Function: RTCCSetBinDay
 *
 * Preconditions: None.
 *
 * Overview: The function verifies a setting day range, translates it
 * into BCD format and writes into _time_chk structure. To write the
 * structure into clock RTCCSet must be called.
 *
 * Input: Day binary value.
 *        int to indicate if this is for the alarm (1) or not(0)
 *
 * Output: Checked BCD value in _time_chk structure.
 *
 ********************************************************************/
void RTCCSetBinDay(unsigned char Day, int forAlarm)
{
	const char MonthDaymax[] =
	//jan feb mar apr may jun jul aug sep oct nov dec
	{  31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
	unsigned Daymax;
	unsigned Month;
	unsigned Year;

    Month = mRTCCDec2Bin(_time_chk.mth);
    Year = mRTCCDec2Bin(_time_chk.yr);

    Daymax = MonthDaymax[Month-1];

    // February has one day more for a leap year
    if(Month == 2)
    if( (Year%4) == 0)
        Daymax++;

    if(Day == 0) Day = Daymax;
    if(Day > Daymax) Day = 1;
	if (forAlarm > 0) {
	    _alarm.day = mRTCCBin2Dec(Day);
	} else {
	    _time_chk.day = mRTCCBin2Dec(Day);
	}
}

/*********************************************************************
 * Function: RTCCSetBinMonth
 *
 * Preconditions: None.
 *
 * Overview: The function verifies a setting month range, translates
 * it into BCD format and writes into _time_chk structure. To write
 * the structure into clock RTCCSet must be called.
 *
 * Input: Month binary value.
 *        int to indicate if this is for the alarm (1) or not(0)
 *
 * Output: Checked BCD value in _time_chk structure.
 *
 ********************************************************************/
void RTCCSetBinMonth(unsigned char Month, int forAlarm)
{
    if(Month < 1) Month = 12;
    if(Month > 12) Month = 1;
	if (forAlarm > 0) {
	    _alarm.mth = mRTCCBin2Dec(Month);
	} else {
	    _time_chk.mth = mRTCCBin2Dec(Month);
	}
}

/*********************************************************************
 * Function: RTCCSetBinYear
 *
 * Preconditions: None.
 *
 * Overview: The function verifies a setting year range, translates it
 * into BCD format and writes into _time_chk structure. To write the 
 * structure into clock RTCCSet must be called.
 *
 * Input: Year binary value.
 *        int to indicate if this is for the alarm (1) or not(0)
 *
 * Output: Checked BCD value in _time_chk structure.
 *
 ********************************************************************/
void RTCCSetBinYear(unsigned char Year, int forAlarm)
{
   if(Year >= 100)  Year = 0;
	if (forAlarm > 0) {
	    _alarm.yr = mRTCCBin2Dec(Year);
	} else {
	    _time_chk.yr = mRTCCBin2Dec(Year);
	}
    // Recheck day. Leap year influences to Feb 28/29.
    RTCCSetBinDay(mRTCCDec2Bin(_time_chk.day), forAlarm);
}


