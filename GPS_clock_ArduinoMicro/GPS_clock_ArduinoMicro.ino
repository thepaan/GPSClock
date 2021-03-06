// GPS clock using test code from Adafruit's GPS and I2C 7-segment LED backpack.
// Original GPS code here: https://github.com/adafruit/Adafruit-GPS-Library
// Original LED Backpack code here: https://github.com/adafruit/Adafruit-LED-Backpack-Library
//
// Big thanks to Lady Ada from Adafruit and driverblock from the Adafruit forums for help getting this going.
//
// Tested and works great with:
// Adafruit GPS with the MTK33x9 chipset ---------> https://www.adafruit.com/products/746
// Arduino Micro -------------------------------
// 4-Digit, 7-Segment Display with I2C Backpack --> https://www.adafruit.com/products/879
// Plastic mounting plate for Ardunio ------------> https://www.adafruit.com/products/275
// Breadboarding wire ----------------------------> https://www.adafruit.com/products/153

// Pick one up today at the Adafruit electronics shop 
// and help support open source hardware & software! -ada

#include <Adafruit_LEDBackpack.h>;
#include <Adafruit_GPS.h>;
#include <Adafruit_GFX.h>
#include <SoftwareSerial.h>;
#include <Wire.h>;
Adafruit_GPS GPS(&Serial1);

int fourdigitTime;
int zuluTime;
Adafruit_7segment
  matrixA = Adafruit_7segment(),
  matrixB = Adafruit_7segment();

// Set GPSECHO to 'false' to turn off echoing the GPS data to the Serial console
// Set to 'true' if you want to debug and listen to the raw GPS sentences
#define GPSECHO false

// this keeps track of whether we're using the interrupt
// off by default!
boolean usingInterrupt = false;

void setup()  
{
  matrixA.begin(0x70);
  matrixB.begin(0x71);
  // connect at 115200 so we can read the GPS fast enough and echo without dropping chars
  // also spit it out
  Serial.begin(115200);
  Serial.println("Adafruit GPS library basic test!");

  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  // uncomment this line to turn on only the "minimum recommended" data
  //GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCONLY);
  // For parsing data, we don't suggest using anything but either RMC only or RMC+GGA since
  // the parser doesn't care about other sentences at this time
  
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  delay(1000);
  // Ask for firmware version
  Serial1.println(PMTK_Q_RELEASE);
}



uint32_t timer = millis();
void loop()                     // run over and over again
{
  // read data from the GPS in the 'main loop'
  char c = GPS.read();
  // if you want to debug, this is a good time to do it!
  if (GPSECHO)
      if (c) Serial.print(c);
  
  // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences! 
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    Serial.println(GPS.lastNMEA());   // this also sets the newNMEAreceived() flag to false
  
    if (!GPS.parse(GPS.lastNMEA()))   // this also sets the newNMEAreceived() flag to false
      return;  // we can fail to parse a sentence in which case we should just wait for another
  }

// first clock adjusted for UTC+9 (Tokyo, Seoul), 24-hour clock, no daylight savings
if (GPS.hour < 15) {
  fourdigitTime = ((GPS.hour + 9) *100)+ GPS.minute;
} else {
  fourdigitTime = ((GPS.hour - 15) *100)+ GPS.minute;   
}

matrixA.print(fourdigitTime);
matrixA.setBrightness(7); // default 15 is too bright
// matrixA.drawColon(true); // works only for 0.56" display
matrixA.writeDigitRaw(2, 0x02); // display only center colon on 1.2" display
// Serial.print("fourdigitTime: "); Serial.println(fourdigitTime); // too much serial output
if (GPS.hour == 15 && GPS.minute < 1) { // display leading zeros
  matrixB.writeDigitNum(1, 0);
  matrixB.writeDigitNum(3, 0);
  matrixB.writeDigitNum(4, 0);
} else if (GPS.hour == 15 && GPS.minute < 10) {
  matrixA.writeDigitNum(1, 0);
  matrixA.writeDigitNum(3, 0);
} else if (GPS.hour == 15 && GPS.minute >= 10) {
  matrixA.writeDigitNum(1, 0);
}
matrixA.writeDisplay();

// second clock adjusted for UTC (Zulu), 24-hour clock, no daylight savings
zuluTime = (GPS.hour *100)+ GPS.minute;

matrixB.print(zuluTime);
matrixB.setBrightness(7);
matrixB.writeDigitRaw(2, 0x02);
if (GPS.hour < 1 && GPS.minute < 1) {
  matrixB.writeDigitNum(1, 0);
  matrixB.writeDigitNum(3, 0);
  matrixB.writeDigitNum(4, 0);
} else if (GPS.hour < 1 && GPS.minute < 10) {
  matrixB.writeDigitNum(1, 0);
  matrixB.writeDigitNum(3, 0);
} else if (GPS.hour < 1 && GPS.minute >= 10) {
  matrixB.writeDigitNum(1, 0);
}
matrixB.writeDisplay();

  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis())  timer = millis();

  // approximately every 2 seconds or so, print out the current stats
  if (millis() - timer > 2000) { 
    timer = millis(); // reset the timer
    
    Serial.print("\nTime: ");
    Serial.print(GPS.hour, DEC); Serial.print(':');
    Serial.print(GPS.minute, DEC); Serial.print(':');
    Serial.print(GPS.seconds, DEC); Serial.print('.');
    Serial.println(GPS.milliseconds);
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    Serial.print("Fix: "); Serial.print((int)GPS.fix);
    Serial.print(" quality: "); Serial.println((int)GPS.fixquality); 
    if (GPS.fix) {
      Serial.print("Location: ");
      Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
      Serial.print(", "); 
      Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
      
      Serial.print("Speed (knots): "); Serial.println(GPS.speed);
      Serial.print("Angle: "); Serial.println(GPS.angle);
      Serial.print("Altitude: "); Serial.println(GPS.altitude);
      Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
    }
  }
}
