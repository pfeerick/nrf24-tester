/*
 *This demo code will show you all functions for
 *Digole Graphic LCD adapter
 */
#define _Digole_Serial_I2C_  //To tell compiler compile the special communication only, 
#define SERIAL_DEBUG 

#include "MAX17043.h"
#include "Wire.h"

MAX17043 batteryMonitor;


//#ifdef SERIAL_DEBUG
#include "printf.h"
//#endif

//all available are:_Digole_Serial_UART_, _Digole_Serial_I2C_ and _Digole_Serial_SPI_
#include <DigoleSerial.h>

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#include <Wire.h>
DigoleSerialDisp mydisp(&Wire,'\x27');  //I2C:Arduino UNO: SDA (data line) is on analog input pin 4, and SCL (clock line) is on analog input pin 5 on UNO and Duemilanove



const unsigned char fonts[] = {6, 10, 18, 51, 120, 123};

struct sensorData {
  sensorData(int iStatus, float iTemp, float iMinTemp) : status(iStatus), temp(iTemp), minTemp(iMinTemp) {}
  sensorData() : status(-1), temp(1023.0), minTemp(1023.0) {}
  int status;
  float temp;
  float minTemp;
} ;

sensorData sensors[2];

sensorData sensor1; //(-1,1023.0,1023.0);
sensorData sensor2; //(-1,1023.0,1023.0);

//double sensorData[2][3] = {{0.0,1023.0,1023.0}}; //status, temp, minTemp //can be expaneded to humid,min_humid, etc

unsigned long lastSensorUpdate;
const long sensorUpdateFrequency = 5000;
boolean firstSensorUpdate = true;

unsigned long lastDisplayUpdate;
const long displayUpdateFrequency = 15000;
boolean firstDisplayUpdate = true;

boolean newSensorData = false;
float t, h, min_T, min_H;

// Set up nRF24L01 radio on SPI bus plus pins 9 & 10
RF24 radio(9,10);

// Single radio pipe address for the 2 nodes to communicate.
const uint64_t pipes[3] = { 0xF0F0F0F0E1LL, 0xF0F0F0F0D2LL, 0xF0F0F0F0D3LL };

void setup() {
//#ifdef SERIAL_DEBUG
  Serial.begin(9600);
  Serial.print("Init!");
  printf_begin();
//#endif

  Wire.begin(); 
  batteryMonitor.reset();
  batteryMonitor.quickStart();

  delay(1000); //delay for max17043 reboot and oled bootup
  
  mydisp.begin();
  mydisp.disableCursor(); //disable cursor, enable cursore use: enableCursor();
  mydisp.clearScreen(); //CLear screen
    
    
  radio.begin();
  radio.setChannel(50);
  radio.setRetries(15,15);
  radio.setPayloadSize(8);
  radio.setDataRate(RF24_250KBPS);  
  
  radio.openWritingPipe(pipes[0]);
  radio.openReadingPipe(1,pipes[1]);
  radio.openReadingPipe(2,pipes[2]);

//#ifdef SERIAL_DEBUG
  radio.printDetails();
//#endif

  radio.startListening();
}
void loop() 
{

   float sensor_data;
  unsigned long currentMillis = millis();
  uint8_t sensor = 0;

 if (radio.available(&sensor))
  {
    // Dump the payloads until we've gotten everything
    bool done = false;
    while (!done)
    {
      done = radio.read(&sensor_data, sizeof(sensor_data) );
    }

//#ifdef SERIAL_DEBUG
    Serial.print("New remote data from #");
    Serial.print(sensor);
    Serial.print(" = ");
    Serial.println(sensor_data);
//#endif
    // Fetch the payload,  see if this was the last one.
    switch (sensor)
    {
    case 1:
      if (sensor_data != 1023.0)
      {
        sensor1.status = 1;
      }
      else
      {
        sensor1.status = 0;
      }
      sensor1.temp = sensor_data;
      if (sensor1.minTemp != 1023.0 && sensor1.temp  < sensor1.minTemp) 
      {
        sensor1.minTemp = sensor1.temp;
      }
      break;

    case 2:
      if (sensor_data != 1023.0)
      {
        sensor2.status = 1;
      }
      else
      {
        sensor2.status = 0;
      }
      sensor2.temp = sensor_data;
      if (sensor2.minTemp != 1023.0 && sensor2.temp  < sensor2.minTemp) 
      {
        sensor2.minTemp = sensor2.temp;
      }
      break;
    }  

    //    remote_temp = strtod(remote_data,NULL);
//    newSensorData = true;
  }
//#ifdef SERIAL_DEBUG
  else
  {
    //Serial.println("No radio available");
  }
//#endif

 if (currentMillis - lastDisplayUpdate > displayUpdateFrequency || firstDisplayUpdate)
  {
    if (firstDisplayUpdate)
      firstDisplayUpdate = false;

    if (newSensorData)
      newSensorData = 0;

    float cellVoltage = batteryMonitor.getVCell();
    float stateOfCharge = batteryMonitor.getSoC();

    mydisp.clearScreen();
    mydisp.setFont(fonts[2]);

    mydisp.print("Sensor 1: ");
    if (sensor1.status != -1)
    {    
      mydisp.print(sensor1.temp,1);
    }
    else
    {
      mydisp.print("---");
    }
    mydisp.nextTextLine();      
      
    mydisp.print("Sensor 2: ");
    if (sensor2.status != -1)
    {    
      mydisp.print(sensor2.temp,1);
    }
    else
    {
      mydisp.print("---");
    }
    mydisp.nextTextLine();
    
    mydisp.print("BatSoC: ");
    mydisp.print(stateOfCharge);
    mydisp.print("%");    
    mydisp.nextTextLine();

    mydisp.print("BatVcc: ");
    mydisp.print(cellVoltage);
    mydisp.print("v");    
    //mydisp.nextTextLine();
     
    lastDisplayUpdate = currentMillis;  
  }

}
