#if defined(__AVR_ATtiny84__) || defined(__AVR_ATtiny85__) || (__AVR_ATtiny2313__)
#include <TinyWireM.h>
#define Wire TinyWireM
#else
#include <Wire.h>
#endif
#include "TinyRTCY.h"

#define RTC_CHIP_ADDR 0x68
#define TM_FIELDS 7

TinyRTC::TinyRTC(int i2cDAT, int i2cCLK)
{
	if(i2cDAT==i2cCLK){Wire.begin();} // if no values passed assume default arduino setting
	else{Wire.begin(i2cDAT, i2cCLK);}
   
}

// PUBLIC FUNCTIONS
tmElements_tY TinyRTC::getTM()
{
    tmElements_tY tm;
    read(tm);
    tm.Year = tmYearToCalendar(tm.Year);
    return tm;
}

time_tY TinyRTC::get() // Aquire data from buffer and convert to time_tY
{
    tmElements_tY tm;
    if (read(tm) == false)
        return 0;
    return (makeTime(tm));
}

bool TinyRTC::set(time_tY t)
{
    tmElements_tY tm;
    breakTime(t, tm);
    return write(tm);
}



bool TinyRTC::set(uint8_t hr, uint8_t min, uint8_t sec, uint8_t dy, uint8_t mnth, uint16_t yr)
{
    if (yr > 99)
        yr = yr - 1970;
    else
        yr += 30;

    tmElements_tY tm;

    tm.Year = yr;
    tm.Month = mnth;
    tm.Day = dy;
    tm.Hour = hr;
    tm.Minute = min;
    tm.Second = sec;
    set(makeTime(tm));
}

bool TinyRTC::isRunning()
{    // cleaned up be fenirstartdust
	// start rtc's osculator if it was stopped, the rtc supports pauseing the time output and update this saving power, default state if batt is dead. IF battery is removed it is nessary to add a resistor in place...
 Wire.begin(2,0); // do once to start i2c as master, with no address set for self.
  Wire.beginTransmission(RTC_CHIP_ADDR); // 0x68
  Wire.write((uint8_t)0x00);    // set reg pointer to 00 seconds
  while(Wire.endTransmission() != 0){return false;}  // if it won't respond return false
    // Just fetch the seconds register and check the top bit
  Wire.requestFrom(RTC_CHIP_ADDR, (uint8_t)1); // tell the rtc to send over 1 byte 8 bit value
  if((Wire.read() & 0x80)){Wire.beginTransmission(RTC_CHIP_ADDR);Wire.write((uint8_t)0x00); Wire.write((uint8_t)0x00);Wire.endTransmission();}  
  // set reg to 00 seconds
  // LSB so that hex 80 means 10000000, the 1 means clock is reset from battery failure, and is also halted...i guess for setting time
  // set to 000000000 clock osculator then is active
  return true;
}


bool TinyRTC::read(tmElements_tY &tm)
{
	// Aquire data from the RTC chip in BCD format
    Wire.beginTransmission(RTC_CHIP_ADDR);
    Wire.write((uint8_t)0x00);
    if (Wire.endTransmission() != 0){return false; }
    // request the 7 data fields   (secs, min, hr, dow, date, mth, yr)
    Wire.requestFrom(RTC_CHIP_ADDR, TM_FIELDS);
    if (Wire.available() < TM_FIELDS) {return false;} // when is this true? if clock is stopped does it still give the values?
	tm.Second = Wire.read(); // temp reuse of this var, 
	 if ((tm.Second & 0x80)) {isRunning(); tm.Second=0x00;}// clock is halted someone didn't set running before reading time, failsafe, or this is not the device you are looking for?
    tm.Second = bcd2dec(tm.Second & 0x7f);
    tm.Minute = bcd2dec(Wire.read());
    tm.Hour = bcd2dec(Wire.read() & 0x3f); // mask assumes 24hr clock
    tm.Wday = bcd2dec(Wire.read());
    tm.Day = bcd2dec(Wire.read());
    tm.Month = bcd2dec(Wire.read());
    tm.Year = y2kYearToTm((bcd2dec(Wire.read())));
    return true;
}

bool TinyRTC::write(tmElements_tY &tm)
{
    // To eliminate any potential race conditions,
    // stop the clock before writing the values,
    // then restart it after.
    Wire.beginTransmission(RTC_CHIP_ADDR);
    Wire.write((uint8_t)0x00); // reset register pointer
    Wire.write((uint8_t)0x80); // Stop the clock. The seconds will be REwritten last, it's been written to stop clock bit value
    Wire.write(dec2bcd(tm.Minute));
    Wire.write(dec2bcd(tm.Hour)); // sets 24 hour format
    Wire.write(dec2bcd(tm.Wday));
    Wire.write(dec2bcd(tm.Day));
    Wire.write(dec2bcd(tm.Month));
    Wire.write(dec2bcd(tmYearToY2k(tm.Year)));
    if (Wire.endTransmission() != 0){ return false;}
    // Now go back and set the seconds, starting the clock back up as a side effect
    Wire.beginTransmission(RTC_CHIP_ADDR);
    Wire.write((uint8_t)0x00);      // reset register pointer
    Wire.write(dec2bcd(tm.Second)); // write the seconds, this will also start the clock ocsulator again
    if (Wire.endTransmission() != 0){return false;}
    return true;
}

void TinyRTC::breakTime(time_tY timeInput, tmElements_tY &tm)
{
    // break the given time_tY into time components
    // this is a more compact version of the C library localtime function
    // note that year is offset from 1970 !!!
    uint8_t year;
    uint8_t month, monthLength;
    uint32_t time;
    unsigned long days;

    time = (uint32_t)timeInput;
    tm.Second = time % 60;
    time /= 60; // now it is minutes
    tm.Minute = time % 60;
    time /= 60; // now it is hours
    tm.Hour = time % 24;
    time /= 24;                     // now it is days
    tm.Wday = ((time + 4) % 7) + 1; // Sunday is day 1

    year = 0;
    days = 0;
    while ((unsigned)(days += (LEAP_YEAR(year) ? 366 : 365)) <= time)
    {
        year++;
    }
    tm.Year = year; // year is offset from 1970

    days -= LEAP_YEAR(year) ? 366 : 365;
    time -= days; // now it is days in this year, starting at 0

    days = 0;
    month = 0;
    monthLength = 0;
    for (month = 0; month < 12; month++)
    {
        if (month == 1)
        { // february
            if (LEAP_YEAR(year))
            {
                monthLength = 29;
            }
            else
            {
                monthLength = 28;
            }
        }
        else
        {
            monthLength = monthDays[month];
        }

        if (time >= monthLength)
        {
            time -= monthLength;
        }
        else
        {
            break;
        }
    }
    tm.Month = month + 1; // jan is month 1
    tm.Day = time + 1;    // day of month
}

time_tY TinyRTC::makeTime(const tmElements_tY &tm)
{
    // assemble time elements into time_tY seconds since 1970
    // note year argument is offset from 1970 (see macros in time.h to convert to other formats)
    // previous version used full four digit year (or digits since 2000),i.e. 2009 was 2009 or 9

    int i;
    uint32_t seconds;

    // seconds from 1970 till 1 jan 00:00:00 of the given year
    seconds = tm.Year * (SECS_PER_DAY * 365);
    for (i = 0; i < tm.Year; i++)
    {
        if (LEAP_YEAR(i))
        {
            seconds += SECS_PER_DAY; // add extra days for leap years
        }
    }

    // add days for this year, months start from 1
    for (i = 1; i < tm.Month; i++)
    {
        if ((i == 2) && LEAP_YEAR(tm.Year))
        {
            seconds += SECS_PER_DAY * 29;
        }
        else
        {
            seconds += SECS_PER_DAY * monthDays[i - 1]; //monthDay array starts from 0
        }
    }
    seconds += (tm.Day - 1) * SECS_PER_DAY;
    seconds += tm.Hour * SECS_PER_HOUR;
    seconds += tm.Minute * SECS_PER_MIN;
    seconds += tm.Second;
    return (time_tY)seconds;
}

// Convert Decimal to Binary Coded Decimal (BCD)
uint8_t TinyRTC::dec2bcd(uint8_t num)
{
    return ((num / 10 * 16) + (num % 10));
}

// Convert Binary Coded Decimal (BCD) to Decimal
uint8_t TinyRTC::bcd2dec(uint8_t num)
{
    return ((num / 16 * 10) + (num % 16));
}

TinyRTC RTC = TinyRTC(0, 0);
