/*Sensor library*/
#include <K30_I2C.h>
#include "SparkFun_SCD30_Arduino_Library.h"
/*End of Sensor library*/

/*WiFi Library and Thinkspeak*/
#include "ThingSpeak.h"
#include <WiFi.h>
/*Ens of WiFi Library and Thinkspeak*/

/*General library*/
#include <Wire.h> // Although Wire.h is part of the Ardunio GUI

/*End General library*/

/*Sensor PIN*/
#define PIN 12
#define ANALOGPIN 4
#define ADDR_6713 0x15 // default I2C slave address
/*End of Sensor PIN*/

/*Credientials*/
#define SECRET_SSID "JioFi3_259D0E" // replace MySSID with your WiFi network name
#define SECRET_PASS "k61m12m0u5"

#define SECRET_CH_ID  123456// replace 0000000 with your channel number
#define SECRET_WRITE_APIKEY "deeeeax2exd"
/*End of Credientials*/

/*Variable defining*/
unsigned long duration, th, tl;
int data [4];
int T6CO2ppm;
int ppm;
//Gravity
int adcVal;
//K30
int k30_co2 = 0;
int rc  = 1;
float concentration;

char ssid[] = SECRET_SSID; // your network SSID (name)
char pass[] = SECRET_PASS; // your network password
int keyIndex = 0; // your network key Index number (needed only for WEP)

unsigned long myChannelNumber = SECRET_CH_ID;
const char * myWriteAPIKey = SECRET_WRITE_APIKEY;
int analogPin = 4;

/*END*/

WiFiClient client;
SCD30 airSensor;
K30_I2C k30_i2c = K30_I2C(0x68);
int scd30_ppm;

void setup() {
  Serial.begin(9600);
  WiFi.mode(WIFI_STA);
  ThingSpeak.begin(client);
  Wire.begin ();
  //SCD30
  if (airSensor.begin() == false)
  {
    Serial.println("Air sensor not detected. Please check wiring. Freezing...");
  }
}


void loop() {
  Serial.println("Inside loop");
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(SECRET_SSID);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, pass); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  }

  // start I2C
  Wire.beginTransmission(ADDR_6713);
  Wire.write(0x04); Wire.write(0x13);
  Wire.write(0x8B); Wire.write(0x00);
  Wire.write(0x01);
  // end transmission
  Wire.endTransmission();
  // read report of current gas measurement in ppm
  delay(2000);
  Wire.requestFrom(ADDR_6713, 4); // request 4 bytes from slave

  data[0] = Wire.read();
  data[1] = Wire.read();
  data[2] = Wire.read();
  data[3] = Wire.read();
  //  Serial.print("Func code: "); Serial.print(data[0], HEX);
  //  Serial.print(" byte count: "); Serial.println(data[1], HEX);
  //  Serial.print("MSB: 0x"); Serial.print(data[2], HEX);
  //  Serial.print(" ");
  //  Serial.print("LSB: 0x"); Serial.print(data[3], HEX);
  //  Serial.print(" ");
  T6CO2ppm = ((data[2] * 0xFF ) + data[3]);
  Serial.print(" T6CO2ppm Co2: ");
  Serial.println(T6CO2ppm);
  ///////////////////////////////////////////////////////////////////////////////////////////////
  th = pulseIn(PIN, HIGH, 2008000) / 1000;
  tl = 1004 - th;
  //  Serial.print("th");
  //  Serial.println(th);
  //  Serial.print("tl");
  //  Serial.println(tl);
  ppm = 2000 * (th - 2) / (th + tl - 4);
  Serial.print(" Prana_PPM Co2: ");
  Serial.println(ppm);
  /////////////////////////////////////////////////////////////////////////////////////////////////
  adcVal = analogRead(analogPin);
  float voltage = adcVal * (1.2 / 4095.0);
  float voltageDiference = voltage - 0.4;
  concentration = (voltageDiference * 5000.0) / 1.6;
  //  Serial.print("voltage:");
  //  Serial.print(voltage);
  //  Serial.println("V_first");
  Serial.print("Gravity: ");
  Serial.print(concentration);
  Serial.println("ppm");
  ///////////////////////////////////////////////////////////////////////////////////////////////////
  rc = k30_i2c.readCO2(k30_co2);
  if (rc == 0) {
    //    Serial.print("Succesful reading\n");
    Serial.print(" k30_co2: ");
    Serial.print(k30_co2);
    Serial.println(" ppm");
  } else {
    Serial.print("Failure to read sensor\n");
  }
  ////////////////////////////////////////////////////////////////////////////////////////////////////
  if (airSensor.dataAvailable())
  {
    scd30_ppm = airSensor.getCO2();
    Serial.print("scd_co2(ppm):");
    Serial.println(scd30_ppm);
  }
  else
    Serial.println("Waiting for new data");
  ////////////////////////////////////////////////////////////////////////////////////////////////////
Serial.println("End of loop");
  ThingSpeak.setField(2, T6CO2ppm);
  ThingSpeak.setField(3, concentration);
  ThingSpeak.setField(1, k30_co2);
  ThingSpeak.setField(4, ppm);
  ThingSpeak.setField(5, scd30_ppm);
  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  delay(3000);

}
