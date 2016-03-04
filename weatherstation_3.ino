//Weather station ino V1 date 28/2/16
/* to Add
Wind direction
Soil temperature
Soil moisture
*/
#include <Adafruit_Sensor.h>//
#include <Adafruit_TSL2561_U.h>
#include "DHT.h"
#include <Wire.h>
#include "RTClib.h"
#include <SPI.h>
#include <SD.h>
#include <SFE_BMP180.h>

#define DHTPIN 3 //D2 pin on arduino 
#define DHTTYPE DHT22 //defines which dht sensor you are using, either 11 or 22.
#define ALTITUDE 77.0 //Altitude of shepshed taken from google.
SFE_BMP180 pressure;
RTC_DS1307 RTC;
DHT dht(DHTPIN, DHTTYPE);
Adafruit_TSL2561_Unified tsl = Adafruit_TSL2561_Unified(TSL2561_ADDR_FLOAT, 12345);
const int chipSelect = 10;//SD card pin
float diameter = 2.75;//for calculating wind speed
float mph;// variable to stor wind speed
int half_revolutions = 0;// storing half revolutions
int rpm = 0;// variable for calculating wind speed
unsigned long lastmillis = 0;
void configureSensor(void)//setting config for TSL2561
{
   tsl.enableAutoRange(true);// enables sensor to change automatically depending on light intensity 
  
   tsl.setIntegrationTime(TSL2561_INTEGRATIONTIME_402MS);// slowest sampling time giving out a hi res reading.

 
}

void setup () {


  Serial.begin(9600);
  Wire.begin();
  RTC.begin();
  SD.begin(10);
  dht.begin();
  pinMode(2, INPUT_PULLUP );//Anenometer set as input
  attachInterrupt(0, rpm_fan, FALLING);// interrupt for pin 2
  if (! RTC.isrunning()) {
   digitalWrite(5,HIGH);// led lights if clock failed to start
  }
  if (pressure.begin()) {
    digitalWrite(6,HIGH); //led lights if pressure sensor begin was successful
  }
configureSensor();
//RTC.adjust(DateTime(__DATE__, __TIME__));// used to automatically set the time upon upload of sketch


}

// below is the fuction that logs data every minute
void loggerGetData (){
  DateTime now = RTC.now();
  char status;
  double T,P,p0,a;
  status = pressure.startTemperature();
  if (status !=0){
    delay(status);
  }
  status = pressure.getTemperature(T);
  status = pressure.startPressure(3);
  if (status !=0) {
    delay (status);
  }
  status = pressure.getPressure(P,T);
  p0 = pressure.sealevel(P,ALTITUDE);

  float h = dht.readHumidity();
  float t = dht.readTemperature();
  sensors_event_t event;
  tsl.getEvent(&event);

   File dataFile = SD.open("LGR.csv", FILE_WRITE);
    if (dataFile) {
    dataFile.print(event.light);
    dataFile.print(",");
    dataFile.print(h);
    dataFile.print(",");
    dataFile.print(t);
    dataFile.print(",");
    dataFile.print(T,2);
    dataFile.print(",");
    dataFile.print(P,2);
    dataFile.print(",");    
    dataFile.print(p0,2);
    dataFile.print(",");
    dataFile.print(mph);
    dataFile.print(",");
    dataFile.print(now.day(),DEC);
    dataFile.print("-");
    dataFile.print(now.month(),DEC);
    dataFile.print(",");
    dataFile.print(now.hour(),DEC);
    dataFile.print(":");
    dataFile.println(now.minute(),DEC);
    dataFile.close();
    }

}
//used for anenometer
void rpm_fan(){
  half_revolutions++; 
}

void loop () {
 DateTime now = RTC.now();
 if (now.second() == 0 ){
  loggerGetData();
 }
 else{
  
 }

//calculate wind speed every second 
if (now.second() != lastmillis ){ 
 detachInterrupt(0);
 rpm = half_revolutions * 30; 
 half_revolutions = 0; // Restart the RPM counter
 lastmillis = now.second(); // Update lastmillis
 attachInterrupt(0, rpm_fan, FALLING); //enable interrupt
 mph = diameter / 12 * 3.14 * rpm * 60 / 5280;
 mph = mph * 2.5; // calibration factor for anemometer accuracy, adjust as necessary
 Serial.println(mph);

 }
 delay (1000);
   
}

