#include <MIDI.h>
#include <Wire.h>
#include <EEPROM.h>

// address for Terabee laser sensor connnected via USB
#define ADDRESS 0x55
#define ELEMENTS(x)   (sizeof(x) / sizeof(x[0]))   // to count elements in an array

const int num_sensors = 3;

typedef struct {    // holds all free parameters (for presets)
  int reverse[num_sensors];
  int active[num_sensors];
  int sensor_range[num_sensors];
  int cc_num[num_sensors];
  int channel[num_sensors];
  int default_value[num_sensors];  // for rubber-band mode
  int min_value[num_sensors];
  int max_value[num_sensors];
} parameters;

parameters params;  // struct that holds all present parameters

// Created and binds the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();

// variables to read midi input
byte type, d1, d2;

// Function prototypes
void sendMIDIcc(int control_num = 22);
void sendRubberBand(int control_num = 22);

// constants won't change. They're used here to set pin numbers:
const int buttonPin1 = 4;     // button for standby
const int buttonPin2 = 5;     // button for elastic-mode
const int ledPin1 =  6;       // elastic-mode LED (green)
const int ledPin2 =  7;       // standby LED (red)

int buttonState1 = HIGH;         // current state of the button (LOW = ON)
int lastButtonState1 = HIGH;     // previous state of the button
int buttonState2 = HIGH;        
int lastButtonState2 = HIGH;    
int waitState = HIGH;
int rubberState = HIGH;

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime1 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay1 = 50;    // the debounce time; increase if the output flickers
unsigned long lastDebounceTime2 = 0;
unsigned long debounceDelay2 = 50;

// define array to read message
int msg[2*num_sensors];         // array size must be = num_sensors*2
int data[num_sensors];        // array size must be = num_sensors
int prev_value[num_sensors];  // array size must be = num_sensors
int value;
bool first_time[num_sensors];  // array size must be = num_sensors

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(31250);  // baud rate of MIDI = 31250 Hz

  MIDI.begin(MIDI_CHANNEL_OMNI);  // Listen to all incoming messages
  
  // initialize LED pins as outputs:
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  // initialize the pushbuttons as inputs:
  pinMode(buttonPin1, INPUT);
  pinMode(buttonPin2, INPUT);

  rubberState = HIGH;  // HIGH voltage means the LED is OFF
  waitState = HIGH;
  digitalWrite(ledPin1, rubberState);
  digitalWrite(ledPin2, waitState);
 
  // initialize first_time and reverse flags
  for(int i=0; i<num_sensors; i++)
  {
      first_time[i] = true;
  }
  
  initialise_preset2();  // initial parameters are hard-coded in the function definition
  delay(3000);

}
void loop() {

  // Logic for the button presses
  //##########################################################

  // read the state of the pushbutton value:
  int reading1 = digitalRead(buttonPin1);
  int reading2 = digitalRead(buttonPin2);

  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (reading1 != lastButtonState1) {
    lastDebounceTime1 = millis();
  }

  if((millis() - lastDebounceTime1) > debounceDelay1) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading1 != buttonState1) {
      buttonState1 = reading1;

      // only toggle the LED if the new button state is HIGH (press and release)
      if (buttonState1 == HIGH) {
        rubberState = !rubberState;
      }
    }    
    
  }  
  // set the LED:
  digitalWrite(ledPin1, rubberState);
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState1 = reading1;

// check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (reading2 != lastButtonState2) {
    lastDebounceTime2 = millis();
  }

  if((millis() - lastDebounceTime2) > debounceDelay2) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading2 != buttonState2) {
      buttonState2 = reading2;

      // only toggle the LED if the new button state is HIGH
      if (buttonState2 == HIGH) {
        waitState = !waitState;
      }
    }    
    
  }  
  // set the LED:
  digitalWrite(ledPin2, waitState);
  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState2 = reading2;  

  //#############################################################################//
  // MAIN CODE for each state (wait, rubber, normal mode)
  
  if(waitState == LOW)  // low state means active
  {
    return;  // waiting mode, controller is idle
  }
  
  Wire.beginTransmission(ADDRESS);
  Wire.write(byte(0xAA));      // write to address 0X55 (DEFAULT)
  Wire.write(byte(0x00));      // request distance frame 
  Wire.write(byte(0xAB));      // read from address 0X55 
  Wire.endTransmission(); 
  
  Wire.requestFrom(ADDRESS,6); // Request the transmitted 6 bytes

 if(Wire.available() == 6)
  {
    for(int i=0; i < num_sensors*2; i++){
      msg[i] = Wire.read();
    }
  }
  
  // Convert 2 bytes message to single int value
  int j = 0;
  for(int i=0; i < num_sensors*2; i += 2)
  {
    data[j] = 255*msg[i] + msg[i+1];
    j += 1;
  }
  
  if(rubberState == LOW) // if Rubber mode is ON
  {
    sendRubberBand();
    mergeMIDI();          
    return;  
  }

  // normal mode
  sendMIDIcc();
  mergeMIDI();
  
  //delay(10); 
}


void checkProgramChange()
{
  if (MIDI.read()) 
  {         // Is there a MIDI message incoming ?
    type = MIDI.getType();
    if(type == 0x0C) 
    {
      // change preset!
            
    }

  }     
}

void mergeMIDI()   // listens to any MIDI in and sends it through
{
    if (MIDI.read()) {         // Is there a MIDI message incoming ?
        type = MIDI.getType();
        d1 = MIDI.getData1();
        d2 = MIDI.getData2();
        MIDI.send(type,d1,d2,1);
    }  
}

void sendMIDIcc(int control_num)
{
  for(int i=0; i < num_sensors; i++)  // loop over sensors
  {
      if(data[i] != -256)  // -256 is infinite distance from sensor (no object detected)
      {
          // map data to MIDI CC range [0,127]
          if(params.reverse[i])
          {
            value = map(constrain(data[i], 40, params.sensor_range[i]), 
                  40, params.sensor_range[i], params.min_value[i], params.max_value[i]);   
          }
          else
          {
            value = map(constrain(data[i], 40, params.sensor_range[i]), 
                  40, params.sensor_range[i], params.max_value[i], params.min_value[i]);    
          }
          
          if(first_time[i])  {
              MIDI.sendControlChange(params.cc_num[i], value, 1);
              prev_value[i] = value;
              first_time[i] = false;          
          }
          else {
            if(value != prev_value[i]) {
                  MIDI.sendControlChange(params.cc_num[i], value, 1);
                  prev_value[i] = value;          
          }   
       } 
     }
  } 
}

void sendRubberBand(int control_num)
{
  for(int i=0; i < num_sensors; i++)  // loop over sensors
  {
      if(data[i] != -256)  // -256 is infinite distance from sensor (no hand detected)
      {
          // map data to MIDI CC range [0,127]
          value = map(constrain(data[i], 40, params.sensor_range[i]), 
                    40, params.sensor_range[i], params.max_value[i], params.min_value[i]);
          if(first_time[i])  {
              MIDI.sendControlChange(params.cc_num[i], value, 1);
              prev_value[i] = value;
              first_time[i] = false;          
          }
          else {
            if(value != prev_value[i]) {
                  MIDI.sendControlChange(params.cc_num[i], value, 1);
                  prev_value[i] = value;          
          }   
       } 
     }
     else
     {
         value = params.default_value[i];
         if(value != prev_value[i]) {
                  MIDI.sendControlChange(params.cc_num[i], value, 1);
                  prev_value[i] = value;          
          }
      } 
  }
}

void initialise_preset1() {
    params.cc_num[0] = 22;  params.cc_num[1] = 23; params.cc_num[2] = 24;
    params.reverse[0] = false; params.reverse[1] = false; params.reverse[2] = false;
    params.sensor_range[0] = 150; params.sensor_range[1] = 150; params.sensor_range[2] = 150; 
    params.default_value[0] = 64; params.default_value[1] = 64; params.default_value[2] = 64;  
    params.min_value[0] = 0; params.min_value[1] = 0; params.min_value[2] = 0;
    params.max_value[0] = 127; params.max_value[1] = 127; params.max_value[2] = 127;
}

void initialise_preset2() {
    params.cc_num[0] = 22;  params.cc_num[1] = 23; params.cc_num[2] = 24;
    params.reverse[0] = false; params.reverse[1] = true; params.reverse[2] = true;
    // (min, max) = (40, 150) sensor range due to terabee sensor characteristics
    params.sensor_range[0] = 50; params.sensor_range[1] = 150; params.sensor_range[2] = 100; 
    params.default_value[0] = 30; params.default_value[1] = 90; params.default_value[2] = 30;  
    // lowest possible is 0, highest possible is 127
    params.min_value[0] = 0; params.min_value[1] = 50; params.min_value[2] = 90;
    params.max_value[0] = 127; params.max_value[1] = 100; params.max_value[2] = 127;
}

void initialise_preset3() {
    params.cc_num[0] = 22;  params.cc_num[1] = 23; params.cc_num[2] = 24;
    params.reverse[0] = false; params.reverse[1] = false; params.reverse[2] = false;
    params.sensor_range[0] = 150; params.sensor_range[1] = 150; params.sensor_range[2] = 150; 
    params.default_value[0] = 64; params.default_value[1] = 64; params.default_value[2] = 64;  
    params.min_value[0] = 0; params.min_value[1] = 0; params.min_value[2] = 0;
    params.max_value[0] = 127; params.max_value[1] = 127; params.max_value[2] = 127;
}
