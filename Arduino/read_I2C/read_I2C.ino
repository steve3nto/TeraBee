#include <Wire.h>
//#include "ArduinoOSC.h"

#define ADDRESS  0x55

//ArduinoOSCSerial osc;

// Function prototypes
void ccmessage(int cn, int value, int cmd = 0xB0);  // Default is CC cmd on channel 0

// define array to read message
int msg[6];
int data[3];
int prev_value[3];
int note;
int value;
bool first_time[3];  // flag for first values computed from sensor

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(31250);  // baud rate of MIDI = 31250 Hz

  // initialize first_time flag
  for(int i=0; i<3; i++)
  {
      first_time[i] = true;  
  }
  
  //osc.begin(Serial, 115200);
  //osc.addCallback("/ard/midi", &callback);
  delay(5000);
}
void loop() {
  // put your main code here, to run repeatedly:
  Wire.beginTransmission(ADDRESS);
  Wire.write(byte(0xAA));      // write to address 0X55 (DEFAULT)
  Wire.write(byte(0x00));      // request distance frame 
  Wire.write(byte(0xAB));      // read from address 0X55 (DEFAULT)
  Wire.endTransmission(); 
  
  Wire.requestFrom(ADDRESS,6); // Request the transmitted 6 bytes

 if(Wire.available() == 6)
  {
    for(int i=0; i < 6; i++){
      msg[i] = Wire.read();
    }
  }
  // Convert 2 bytes to single message
  data[0] = 255*msg[0] + msg[1];
  data[1] = 255*msg[2] + msg[3];
  data[2] = 255*msg[4] + msg[5];

//  Serial.print("################################### \n");
//  for(int i=0; i < 3; i++){
//      Serial.print(data[i]);
//      Serial.println();
//    }

// Send note on messages (TEST)

//  if(data[0] != -256) {
//      note = constrain(data[0] - 25, 10, 90);  // convert sensor data to midi note
//      noteOn(0x90, note , 0x45);   //channel 1 note on event
//      delay(500);
//      noteOn(0x90, note, 0x00);
//      delay(500);     
//    }

  for(int i=0; i < 3; i++)  // loop over sensors
  {
      if(data[i] != -256)  // -256 is infinite distance from sensor (no hand detected)
      {
          value = map(data[i], 40, 350, 127, 0);  // map data to [0,127]
          if(first_time[i])  {
              ccmessage(22+i , value);  // control number from 22 on...
              prev_value[i] = value;
              first_time[i] = false;          
          }
          else {
              if(value != prev_value[i]) {
                  ccmessage(22+i , value);  // control number from 22 on...
                  prev_value[i] = value;          
              }   
          } 
      }
  }

//  OSCMessage m;
//  m.beginMessage();
//
//  m.setOSCAddress();
}

//void callback(OSCMessage& m)
//{
//    //create new osc message
//    OSCMessage msg;
//    msg.beginMessage();
//
//    // read & set same argument
//    msg.setOSCAddress(m.getOSCAddress());
//    msg.addArgInt32(m.getArgAsInt32(0));
//    msg.addArgFloat(m.getArgAsFloat(1));
//    msg.addArgString(m.getArgAsString(2));
//
//    //send osc message
//    osc.send(msg);
//}

// plays a MIDI note. Doesn't check to see that cmd is greater than 127, or that
// data values are less than 127:
void noteOn(int cmd, int pitch, int velocity) {
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
}

void ccmessage(int cn, int value, int cmd)
{
    Serial.write(cmd);
    Serial.write(cn);
    Serial.write(value);
}
