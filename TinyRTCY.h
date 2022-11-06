/*
 * TinyRTC.h - library for DS1307 RTC
 * This library is made to reduce DS1307RTC library code size
 * Author: Alauddin Ansari
 * 2020-04-05
 * https://github.com/AlauddinTheWonder/arduino-tiny_ds1307.git
 * rewrote fixed by FENIR  2022
 */

#ifndef TinyRTCX_h
#define TinyRTCX_h

#define tmYearToCalendar(Y) ((Y) + 1970) // full four digit year
#define CalendarYrToTm(Y) ((Y)-1970)
#define y2kYearToTm(Y) ((Y) + 30)
#define tmYearToY2k(Y) ((Y)-30) // offset is from 2000
#define LEAP_YEAR(Y) (((1970 + (Y)) > 0) && !((1970 + (Y)) % 4) && (((1970 + (Y)) % 100) || !((1970 + (Y)) % 400)))

/* Useful Constants */
#define SECS_PER_MIN ((time_tY)(60UL))
#define SECS_PER_HOUR ((time_tY)(3600UL))
#define SECS_PER_DAY ((time_tY)(SECS_PER_HOUR * 24UL))

static const uint8_t monthDays[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31}; // API starts months from 1, this array starts from 0

typedef unsigned long time_tY;

typedef struct
{
    uint8_t Second;
    uint8_t Minute;
    uint8_t Hour;
    uint8_t Wday; // day of week, sunday is day 1
    uint8_t Day;
    uint8_t Month;
    uint16_t Year; // offset from 1970;
} tmElements_tY;



// library interface description
class TinyRTC
{
    public:
        TinyRTC(int i2cDAT, int i2cCLK);
        static tmElements_tY getTM();
        static time_tY get();
        static bool set(time_tY t);
        static bool set(uint8_t hr, uint8_t min, uint8_t sec, uint8_t day, uint8_t month, uint16_t yr);
		static bool isRunning();
        static bool chipPresent() { return isRunning(); } // same as isrunning
		static void breakTime(time_tY time, tmElements_tY &tm); // break time_tY secondssince1970  into elements
        static time_tY makeTime(const tmElements_tY &tm);       // convert time elements into time_tY secondssince1970 

    private:
        static bool read(tmElements_tY &tm);
        static bool write(tmElements_tY &tm);

        static uint8_t dec2bcd(uint8_t num);
        static uint8_t bcd2dec(uint8_t num);
};

#ifdef RTC
    #undef RTC // workaround for Arduino Due, which defines "RTC"...
#endif

extern TinyRTC RTC;

#endif
