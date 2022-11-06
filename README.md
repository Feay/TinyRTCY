# arduino-tiny_rtc
TinyRTCY library. Rewritten and from TinyRTC, DS1307RTC, and Time lib. Very tiny code to control the RTC DS1307 and DS3231 module.
NPT internet atomic clock time setter in example, works on esp8266 =) 

### Type Def
```
> unsigned long time_t
> tmElements_t {
    uint8_t Second;
    uint8_t Minute;
    uint8_t Hour;
    uint8_t Wday; // day of week, sunday is day 1
    uint8_t Day;
    uint8_t Month;
    uint16_t Year; // offset from 1970;
} tmElements_t tm
```
// tmElements_t being used as tm.elements in a sketch

### Available functions
```
>   getTM(); fills tmElements_t as tm.elements, and so on, see the struct tmElements_t
>   get(); // returns secondssince1970 as a long, unit_32
>   bool set(secondssince1970);
>   bool set(int Hour, int Minute, int Second, int Day, int Month, int Year);
>	TinyRTC(int i2cDAT, int i2cCLK); // could be left as TinyRTC(); for default, needed if not already using wire.being(); elsewhere
>	bool isRunning(); should always be used once, starts clock osc if it was stopped, returns false if wire transmission fails
>	bool chipPresent() { return isRunning(); } // same as isrunning
>	void breakTime(secondssince1970); // break time_tY secondssince1970  into elements fills tmElements_t as tm.elements returns nothing
>	makeTime(tmElements_tY);       // convert time elements into time_tY secondssince1970 returns a value



```

#[See example](https://github.com/Feay/TinyRTCY/blob/main/examples/TinyRTCY_Example/Fen.ESP.RTCset.ino)# 
