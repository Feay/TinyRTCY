#include <TinyRTCY.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>

/* using as aliases*/
WiFiUDP Udp;
//TinyRTCY RTC;
/* wifi net vars*/
const char ssid[] = "sex";  //  your network SSID (name)
const char pass[] = "";       // your network password
// NTP Server:
IPAddress timeServer(129, 6, 15, 28); // 129.6.15.28 a server from the usa pool
unsigned int localPort = 8888;  // local port to listen for UDP packets
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets  should it be volatile? 
/* time vars*/
//#define postimezone // uncomment for positive timezones
const int timeZone = 8; //always positive int in this case, math is swapped and inserted below via define
#ifndef postimezone
#define mytoken secsSince1970 = secsSince1970-(timeZone * SECS_PER_HOUR);
#else
#define mytoken secsSince1970 = secsSince1970+(timeZone * SECS_PER_HOUR);
#endif

uint32_t secsSince1970;

void setup() {
  Serial.begin(9600);
  Serial.println("Started ");
  delay(250);
  Serial.print("Wifi:");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.print("my IP is:");
  Serial.println(WiFi.localIP());
  Serial.print("Starting UDP ");
  Udp.begin(localPort); // this is the reciving port, not the port we talk to ntp servers with, that is 123
  Serial.print("local port:");
  Serial.println(Udp.localPort());
  Serial.print("TimeZone var:-"); Serial.print(timeZone);Serial.print("NTP server:"); Serial.print(timeServer);
  connectDS1307();
}

void loop() {
  /*-------- looop and compare time ----------*/
  tmElements_tY tm = RTC.getTM(); // must set time before this??? or run this to turn on clock after set time??
  
  Serial.print("RTC: ");
  Serial.print(tm.Year);
  Serial.print("-");
  Serial.print(tm.Month);
  Serial.print("-");
  Serial.print(tm.Day);
  Serial.print("  ");
  Serial.print(tm.Hour);
  Serial.print(":");
  Serial.print(tm.Minute);
  Serial.print(":");
  Serial.print(tm.Second);
  Serial.println();
  
  while(getNtpTime()==false){;} // wait till we get an response
  Serial.print("NTP: ");
  Serial.print(secsSince1970); // so we can manually double check our math was corrent
  RTC.breakTime(secsSince1970, tm);
  // to use tm.whatever a rtc.getTM or breaktime or maketime  must be called from the same function as using tm. using a univeral print function where we call one of these fucntion then call that printer helper function we made will fail
  Serial.print("-");
  Serial.print(tm.Year);
  Serial.print("-");
  Serial.print(tm.Month);
  Serial.print("-");
  Serial.print(tm.Day);
  Serial.print("  ");
  Serial.print(tm.Hour);
  Serial.print(":");
  Serial.print(tm.Minute);
  Serial.print(":");
  Serial.print(tm.Second);
  Serial.println();
  delay(10000);
}
/*-------- RTC code ----------*/
void connectDS1307() {
//  note-too-self check varants file first, arduino ide is nonsensical pin 0 == D3. pin 2 == D4 on nodelua WTF 
  TinyRTC(2, 0); // initalize wire begin, set d4 data and d3 clock TinyRTC(int i2cDAT, int i2cCLK) if yo forget this atleast use wire begin once else you will be lost
  while (!RTC.isRunning()) {
    Serial.println("RTC not talking");
    Serial.println("Retrying...");
    delay(1000); // should only get here if wire transsmission came back other than 0, meaning some error occured ie bad wiring, bad rtc, rtc with no battery, or you forgot wire begin statment to start i2c
  }

  Serial.println("DS1307 RTC is connected, setting TIME");
    while(getNtpTime()==false){;} // wait
      // success! we have ntp time set the rtc now
    RTC.set(secsSince1970);
    Serial.println("TIME IS SET"); //
    Serial.println(secsSince1970);
//  RTC.set(21, 27, 10, 4, 4, 2020); // (Hour, Min, Sec, Date, Month, Year)
//  RTC.set(1586016902); // either work C++ allows two functions with diffrent prams same name
}

/*-------- NTP code ----------*/

bool getNtpTime()
{ 
  Serial.println("Transmiting NTP Request");
  while (Udp.parsePacket() > 0) ; // discard any previously received packets 
  sendNTPpacket(timeServer);
  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      // convert four bytes starting at location 40 to a long integer
      //  ntp returns since 1900 ephoc, below code seems to adjust to 1970 epoch we need
      secsSince1970 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1970 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1970 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1970 |= (unsigned long)packetBuffer[43];
      secsSince1970 = secsSince1970 - 2208988800UL;
      // secsSince1970 = secsSince1970-(timeZone * SECS_PER_HOUR);
		mytoken;
      return true;
    }
  }
  Serial.println("No NTP Response :-( please wait");
  return false;
}

// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:                 
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
