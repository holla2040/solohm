#include <Wire.h>

byte	rtc_bcd[7]; 

// from https://github.com/sridharrajagopal/DSRTCLib
// indices within the rtc_bcd[] buffer

#define DSRTCLib_TR     0x10
#define DSRTCLib_SEC	0
#define DSRTCLib_MIN	1
#define DSRTCLib_HR	2
#define DSRTCLib_DOW	3
#define DSRTCLib_DATE     4
#define DSRTCLib_MTH	5
#define DSRTCLib_YR	6

#define DSRTCLib_BASE_YR		2000

#define DSRTCLib_CTRL_ID		B1101000

// Define register bit masks

#define DSRTCLib_CLOCKHALT	B10000000

#define DSRTCLib_LO_BCD		B00001111
#define DSRTCLib_HI_BCD		B11110000

#define DSRTCLib_HI_SEC		B01110000
#define DSRTCLib_HI_MIN		B01110000
#define DSRTCLib_HI_HR		B00110000
#define DSRTCLib_LO_DOW		B00000111
#define DSRTCLib_HI_DATE		B00110000
#define DSRTCLib_HI_MTH		B00110000
#define DSRTCLib_HI_YR		B11110000

#define DSRTCLib_ARLM1		0x07
#define DSRTCLib_ARLM1_LO_SEC	B00001111
#define DSRTCLib_ARLM1_HI_SEC	B01110000
#define DSRTCLib_ARLM1_LO_MIN	B01110000
#define DSRTCLib_ARLM1_HI_MIN	B00001111

#define DSRTCLib_SP			0x0E
#define	DSRTCLib_SP_EOSC		B10000000
#define	DSRTCLib_SP_RS2		B00010000
#define	DSRTCLib_SP_RS1		B00001000
#define	DSRTCLib_SP_INTCN		B00000100
#define	DSRTCLib_SP_A2IE		B00000010
#define	DSRTCLib_SP_A1IE		B00000001

#define DSRTCLib_STATUS		0x0F
#define DSRTCLib_STATUS_OSF	B10000000
#define DSRTCLib_STATUS_A2F	B00000010
#define DSRTCLib_STATUS_A1F	B00000001

// Cumulative number of days elapsed at the start of each month, assuming a normal (non-leap) year.
const unsigned int monthdays[] = {
  0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};


void writeTime(void)
{
  Wire.beginTransmission(DSRTCLib_CTRL_ID);
  Wire.write((uint8_t)0x00); // reset register pointer
  for(int i=0; i<7; i++)
  {
    Wire.write(rtc_bcd[i]);
  }
  Wire.endTransmission();

  // clear the Oscillator Stop Flag
  setRegister(DSRTCLib_STATUS, getRegister(DSRTCLib_STATUS) & !DSRTCLib_STATUS_OSF);
}


void setRegister(unsigned char registerNumber, unsigned char value)
{
  Wire.beginTransmission(DSRTCLib_CTRL_ID);
  Wire.write(registerNumber); // set register pointer
  Wire.write(value);
  Wire.endTransmission();
}

unsigned char getRegister(unsigned char registerNumber)
{
  Wire.beginTransmission(DSRTCLib_CTRL_ID);
  Wire.write(registerNumber);
  Wire.endTransmission();
  Wire.requestFrom(DSRTCLib_CTRL_ID, 1);
  return Wire.read();
}
byte bin2bcd(byte v)
{
  return ((v / 10)<<4) + (v % 10);
}

byte bcd2bin(byte v)
{
  return (v&0x0F) + ((v>>4)*10);
}



void epoch_seconds_to_date(unsigned long seconds_left)
{
  // This routine taken from Dallas/Maxim application note 517
  // http://www.maxim-ic.com/app-notes/index.mvp/id/517
  // Arn't the fastest thing, but it produces correct results.

  // NOTE: The earliest date that can be represented by the DS1337 & DS1339 is 1/1/2000 (946684800 in Unix epoch seconds).
  // Passing an earlier Unix time stamp will fail quietly here (produce a date of 0/0/00), 
  // which will probably make your application angry.

  // ALSO NOTE: This has been optimized some to minimize redundant variables, with the side-effect
  // of making it much harder to understand. Please refer to the original appnote above
  // if you are trying to learn from it :-)


  //unsigned long hour;
  //unsigned long day;
  //unsigned long minute;
  //unsigned long second;
  unsigned long month;
  //unsigned long year;

  unsigned long seconds_left_2;
  //unsigned long whole_minutes;
  //unsigned long whole_hours;
  //unsigned long whole_days;
  //unsigned long whole_days_since_1968;
  unsigned long leap_year_periods;
  unsigned long days_since_current_lyear;
  //unsigned long whole_years;
  unsigned long days_since_first_of_year;
  unsigned long days_to_month;
  //unsigned long day_of_week;

  if(seconds_left >= 946684800)
  {
    seconds_left -= 946684800; // correct for difference between DS1337/DS1339 and UNIX epochs.

    seconds_left_2 = seconds_left / 60; // seconds_left_2 = "whole_minutes"
    rtc_bcd[DSRTCLib_SEC] = bin2bcd(seconds_left - (60 * seconds_left_2));                 // leftover seconds

    seconds_left = seconds_left_2 / 60; // seconds_left = "whole_hours"
    rtc_bcd[DSRTCLib_MIN] = bin2bcd(seconds_left_2 - (60 * seconds_left));            // leftover minutes

    seconds_left_2 = seconds_left / 24; //seconds_left_2 = "whole_days"
    rtc_bcd[DSRTCLib_HR] = bin2bcd(seconds_left - (24 * seconds_left_2));         // leftover hours

    //whole_days_since_1968 = whole_days;// + 365 + 366;	// seconds_left_2 = "whole_days" = "whole_days_since_1968"
    leap_year_periods = seconds_left_2 / ((4 * 365) + 1);

    days_since_current_lyear = seconds_left_2 % ((4 * 365) + 1);

    // if days are after a current leap year then add a leap year period
    if ((days_since_current_lyear >= (31 + 29))) {
      leap_year_periods++;
    }
    seconds_left = (seconds_left_2 - leap_year_periods) / 365; // seconds_left = "whole_years"
    days_since_first_of_year = seconds_left_2 - (seconds_left * 365) - leap_year_periods;

    if ((days_since_current_lyear <= 365) && (days_since_current_lyear >= 60)) {
      days_since_first_of_year++;
    }
    //year = seconds_left; // + 68;


    // seconds_left = "year"
    //seconds_left_2 = "month"
    // walk across monthdays[] to find what month it is based on how many days have passed
    //   within the current year
    month = 13;
    days_to_month = 366;
    while (days_since_first_of_year < days_to_month) {
      month--;
      days_to_month = monthdays[month-1];
      if ((month > 2) && ((seconds_left % 4) == 0)) {
        days_to_month++;
      }
    }

    rtc_bcd[DSRTCLib_DATE] = bin2bcd( days_since_first_of_year - days_to_month + 1);

    rtc_bcd[DSRTCLib_DOW] = bin2bcd((seconds_left_2  + 4) % 7);


    //rtc_bcd[DSRTCLib_SEC] = bin2bcd(second);
    //rtc_bcd[DSRTCLib_MIN] = bin2bcd(minute);
    //rtc_bcd[DSRTCLib_HR] = bin2bcd(hour);
    //rtc_bcd[DSRTCLib_DATE] = bin2bcd(day);
    //rtc_bcd[DSRTCLib_DOW] = bin2bcd(day_of_week);
    rtc_bcd[DSRTCLib_MTH] = bin2bcd(month);
    rtc_bcd[DSRTCLib_YR] = bin2bcd(seconds_left);
  }
  else
  {
    // else: "invalid" (< year 2000) epoch format.
    // 'Best' way to handle this is to zero out the returned date. 

    rtc_bcd[DSRTCLib_SEC] = 0; //0x00 binary = 0x00 BCD
    rtc_bcd[DSRTCLib_MIN] = 0;
    rtc_bcd[DSRTCLib_HR] = 0;
    rtc_bcd[DSRTCLib_DATE] = 0;
    rtc_bcd[DSRTCLib_DOW] = 0;
    rtc_bcd[DSRTCLib_MTH] = 0;
    rtc_bcd[DSRTCLib_YR] = 0;
  }

}

unsigned char time_is_set()
{
  // Return TRUE if Oscillator Stop Flag is clear (osc. not stopped since last time setting), FALSE otherwise
  byte asdf = ((getRegister(DSRTCLib_STATUS) & DSRTCLib_STATUS_OSF) == 0);
  return asdf;
}


void setup()
{
  Wire.begin();
  Serial.begin(115200);
  Serial.println("begin solohm_rtc");

  setRegister(DSRTCLib_SP, getRegister(DSRTCLib_SP) & !DSRTCLib_SP_EOSC); //start RTC
  setRegister(DSRTCLib_TR,0xA6); // no diode, 2k resistor;

//     epoch_seconds_to_date(1421908160L);
//    writeTime();


  if(!time_is_set()) // set a time, if none set already...
  {
    Serial.print("Clock not set. ");
    epoch_seconds_to_date(1421906711L);
    writeTime();
  }

}

void loop()
{ 

  Wire.beginTransmission(DSRTCLib_CTRL_ID);
  Wire.write((uint8_t)0x00);
  Wire.endTransmission();

  // request the 7 bytes of data    (secs, min, hr, dow, date. mth, yr)
  Wire.requestFrom(DSRTCLib_CTRL_ID, 7);

  for(int i=0; i<7; i++)
  {
    if (Wire.available()) {
      rtc_bcd[i] = Wire.read();
    }
  }

  for(int i=6; i>=0; i--)
  {
    Serial.print(bcd2bin(rtc_bcd[i]),DEC);
    Serial.print(" ");
  }
  Serial.println();

  delay(1000);
}









