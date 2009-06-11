// Header information for Modbus functions

// This is the size of the buffer that will be used
// for interactions with 
#define MODBUS_SIZE		253

// The function to read the actual velocity in mm/s (register 3000)
float readActualVelocity(void);

// The function to read the flow rate in l/s(register 3002)
float readFlowRate(void);

// The function to read the insulation value (register 3004)
float readInsulationValue(void);

// The function to read the flow as a percent of Qn (register 3012)
float readFlowratePercentValue(void);

// The function to read totalizer 1 integer value in liters x100 (register 3017)
long readTotalizer1Integer(void);

// The function to read totalizer 2 integer value in liters x100 (register 3021)
long readTotalizer2Integer(void);

// The function to read the units for total flow (register 216)
void readTotalFlowUnits(unsigned char[]);

// The function to read the value for Qn->nominal flow (register 226)
float readQn(void);

// This is the function to read the actual date and time of the
// flow meter.  The unsigned char array that is input will be used
// to record a string of the format yyyy-MM-ddThh:mm:ss.  This
// information is read from register 3033.
void readActualDateAndTime(unsigned char[]);

// This is the function to write (set) the date and time of the
// flow meter.
void setActualDateAndTime(unsigned char[]);

// This is the function to read the last date of calibration from
// the flow meter.  This information is read from register 230.
void readCalDateAndTime(unsigned char[]);

// The function to read the calibration factor (register 228)
float readCalibrationFactor(void);

// The function to read the number of hours since first power up
// (register 80)
unsigned long readOperatingHoursSincePowerUp(void);

// The function to read the number of power ups since first power up
// (register 366)
unsigned int readNumberOfPowerUps(void);

// The function to read the maximum flow rate seen (register 407)
float readHighestFlowRate(void);

// This is the function to read the date that the maximum flow seen
// was recorded on.  This information is read from register 409.
void readHighestFlowDateAndTime(unsigned char[]);

// The function to read the lowest flow rate measured (register 412)
float readLowestFlowRate(void);

// The function to read the date when the lowest flow rate was recorded
// (register 414)
void readLowestFlowDateAndTime(unsigned char[]);

// The function to read the highest consumption over a day
// (register 417)
float readHighestDayConsumption(void);

// The function to read the date of the highest consumption
// (register 419)
void readHighestDayConsumptionDateAndTime(unsigned char[]);

// The function to read the cutoff flow rate which is the rate
// that anything less will be reported as 0 (register 239)
float readLowFlowCutoff(void);

// The function to read the units of flow rate (register 210)
void readFlowRateUnits(unsigned char[]);

// The function to read the temperature of the transmitter (register 3042)
float readTransmitterTemp(void);

// The function to read the actual battery capacity (register 3030)
unsigned char readBatteryCapacity(void);

// The function to read the power status (register 3031)
unsigned char readPowerStatus(void);

// The function to read the fault status (register 3016)
unsigned int readFaultStatus(void);

// The function to read the communication module type (register 822)
unsigned char readCommModuleType(void);

// The function to read the date of the last data log entry (register 476)
void readLastLogDate(unsigned char []);

// ----------------------------------------------------------------------
// NOTE: Methods below here are not tested
// ----------------------------------------------------------------------
// The function to read the fractional portion of the total flow (register 3017)
long readTotalizer1Fraction(void);

// The function to read the fractional portion of the total flow (register 3021)
long readTotalizer2Fraction(void);

// Read the device address (register 528)
unsigned int readDeviceAddress(void);

// Read the product ID (should be 10779 for MAG 8000) (register 79)
unsigned int readProductID(void);

// Read the number of parity errors (Address 500)
unsigned int getNumberOfParityErrors();

// Read the baud rate as unsigned long (address 514)
unsigned long getBaudRateAsUnsignedLong();

// Read the current baud rate (address 529)
long getBaudRate();

// Read the parity/framing settings (address 530)
void getParityFraming();

// Check the running status (address 601)
int isRunning(void);

// Get the name of the manufacturer (607-612)
void getManufacturerName(void);

// The function to read the temperature of the sensor (address 3006-3007)
float * readSensorTemperature(void);

// The function to read the slave ID (Modbus command 17)
unsigned char * reportSlaveID(void);
