#include <MIDI.h>
#include <Wire.h>
#include <EEPROM.h>

// address for Terabee laser sensor (serial communication)
#define ADDRESS 0x55   
#define ELEMENTS(x)   (sizeof(x) / sizeof(x[0]))   // to count elements in an array

const int num_sensors = 3;

typedef struct {    // holds all free parameters (for presets)
  bool reverse[num_sensors];
  bool switchMode[num_sensors];
  int activePreset;
  int waitState; 
  int rubberState; 
  int activeState[num_sensors];   // state of the buttons in hold mode
  int presetState[num_sensors];  // state of the buttons in normal mode
  int sensor_range[num_sensors];
  int cc_num[num_sensors];
  int channel[num_sensors];
  int default_value[num_sensors];  // for rubber-band mode
  int min_value[num_sensors];
  int max_value[num_sensors];
} parameters_t;

// Created and binds the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();

// variables to read midi input
byte type, d1, d2;

// struct that holds all current parameters
parameters_t params;  

// constants won't change. They're used here to set pin numbers:
const int buttonPin1 = 4;     // button for standby
const int buttonPin2 = 10;     // button for elastic-mode
const int buttonPin3 = 9;
const int buttonPin4 = 8;
const int buttonPin5 = 5;  // the buttons for the 3 presets / sensor activation
const int ledPin1 =  6;       // rubber band mode LED (green)
const int ledPin2 =  13;       // standby LED (red)
const int ledPin3 =  12;       
const int ledPin4 =  11;       
const int ledPin5 =  7;

int buttonState1 = HIGH;         // current state of the button (LOW = ON)
int lastButtonState1 = HIGH;     // previous state of the button
int buttonState2 = HIGH;        
int lastButtonState2 = HIGH;
int buttonState3 = HIGH;        
int lastButtonState3 = HIGH;
int buttonState4 = HIGH;        
int lastButtonState4 = HIGH;
int buttonState5 = HIGH;        
int lastButtonState5 = HIGH;          
bool check1, check2, check3;  // to check if preset buttons have been pressed

// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int.
unsigned long lastDebounceTime1 = 0;  // the last time the output pin was toggled
unsigned long debounceDelay1 = 50;    // the debounce time; increase if the output flickers
unsigned long lastDebounceTime2 = 0;
unsigned long debounceDelay2 = 50;
unsigned long lastDebounceTime3 = 0;
unsigned long debounceDelay3 = 50;
unsigned long lastDebounceTime4 = 0;
unsigned long debounceDelay4 = 50;
unsigned long lastDebounceTime5 = 0;
unsigned long debounceDelay5 = 50;

// Hold time
unsigned long holdTime = 500;
unsigned long holdStart;
bool holdPressed = false;

// define array to read message
int msg[2*num_sensors];         // array size must be = num_sensors*2
int data[num_sensors];        // array size must be = num_sensors
int prev_value[num_sensors];  // array size must be = num_sensors
int value;
bool first_time[num_sensors];  // array size must be = num_sensors

// Define rubber-band defaults for the 3 sensors
int default_value[] = {63, 63, 63};

// define upper limit for each sensor (how far shall the hand go?)
int sensor_range[] = {200, 200, 200};


void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(31250);  // baud rate of MIDI = 31250 Hz

  MIDI.begin(MIDI_CHANNEL_OMNI);  // Listen to all incoming messages
  
  // initialize LED pins as outputs:
  pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  pinMode(ledPin3, OUTPUT);
  pinMode(ledPin4, OUTPUT);
  pinMode(ledPin5, OUTPUT);
  // initialize the pushbuttons as inputs:
  pinMode(buttonPin1, INPUT_PULLUP);
  pinMode(buttonPin2, INPUT_PULLUP);
  pinMode(buttonPin3, INPUT_PULLUP);
  pinMode(buttonPin4, INPUT_PULLUP);
  pinMode(buttonPin5, INPUT_PULLUP);

  // Load params from EEPROM ################# TO DO ###################
  params.rubberState = HIGH;    // reversed logic
  params.waitState = HIGH;
  params.activeState[0] = LOW;  
  params.activeState[1] = LOW;  // all active by default
  params.activeState[2] = LOW;

  // Choose preset 1, 2 or 3 
  params.activePreset = 1;   
  Upload_preset_state(); 
  
  // light leds according to params
  digitalWrite(ledPin1, params.rubberState);
  digitalWrite(ledPin2, params.waitState);
  digitalWrite(ledPin3, params.presetState[0]);
  digitalWrite(ledPin4, params.presetState[1]);
  digitalWrite(ledPin5, params.presetState[2]);
 
  // initialize first_time and reverse flags
  for(int i=0; i<3; i++)
  {
      first_time[i] = true;
  }


  delay(3000);

}
void loop() {

  // Logic for the button presses
  //##########################################################

  // Check if HOLD MODE gets activated   ###### TO FIX ##########
  // ButtonHold(buttonPin1, ledPin2, holdStart, holdTime, holdPressed);
  
  // read the state of the pushbutton value:
  int reading1 = digitalRead(buttonPin1);
  int reading2 = digitalRead(buttonPin2);
  int reading3 = digitalRead(buttonPin3);
  int reading4 = digitalRead(buttonPin4);
  int reading5 = digitalRead(buttonPin5); 

  // WAIT BUTTON
  buttonPress(reading1, lastButtonState1, params.waitState, ledPin1,
                buttonState1, lastDebounceTime1, debounceDelay1);
  
  // RUBBER BAND BUTTON
  buttonPress(reading2, lastButtonState2, params.rubberState, ledPin2,
                buttonState2, lastDebounceTime2, debounceDelay2);
  
  // PRESET BUTTON 1
  check1 = buttonPress(reading3, lastButtonState3, params.presetState[0], ledPin3,
                buttonState3, lastDebounceTime3, debounceDelay3);
  if(check1)
  {
    initialise_preset1();
    // OPTIONAL, FLASH ACTIVE SENSORS
    digitalWrite(ledPin3, params.activeState[0]);
    digitalWrite(ledPin4, params.activeState[1]);
    digitalWrite(ledPin5, params.activeState[2]);
    delay(500);
    
    digitalWrite(ledPin3, params.presetState[0]);
    digitalWrite(ledPin4, params.presetState[1]);
    digitalWrite(ledPin5, params.presetState[2]);
  }
  
  // PRESET BUTTON 2
  check2 = buttonPress(reading4, lastButtonState4, params.presetState[1], ledPin4,
                buttonState4, lastDebounceTime4, debounceDelay4);
  if(check2)
  {
    initialise_preset2();

    // OPTIONAL, FLASH ACTIVE SENSORS
    digitalWrite(ledPin3, params.activeState[0]);
    digitalWrite(ledPin4, params.activeState[1]);
    digitalWrite(ledPin5, params.activeState[2]);
    delay(500);
    
    digitalWrite(ledPin3, params.presetState[0]);
    digitalWrite(ledPin4, params.presetState[1]);
    digitalWrite(ledPin5, params.presetState[2]);
  }

  // PRESET BUTTON 3
  check3 = buttonPress(reading5, lastButtonState5, params.presetState[2], ledPin5,
                buttonState5, lastDebounceTime5, debounceDelay5);
  if(check3)
  {
    initialise_preset3();
    // OPTIONAL, FLASH ACTIVE SENSORS
    digitalWrite(ledPin3, params.activeState[0]);
    digitalWrite(ledPin4, params.activeState[1]);
    digitalWrite(ledPin5, params.activeState[2]);
    delay(500);
    
    digitalWrite(ledPin3, params.presetState[0]);
    digitalWrite(ledPin4, params.presetState[1]);
    digitalWrite(ledPin5, params.presetState[2]);
  }

  
  //#############################################################################//
  // MAIN CODE for each state (wait, rubber, normal mode)
  
  if(params.waitState == LOW)  // low state means active
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
  
  // Convert 2 bytes to single message 
  int j = 0;
  for(int i=0; i < num_sensors*2; i += 2)
  {
    data[j] = 255*msg[i] + msg[i+1];
    j += 1;
  }
  

  if(params.rubberState == LOW)
  {
    sendRubberBand();
    mergeMIDI();          
    return;  
  }
  
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

void sendMIDIcc()
{
  for(int i=0; i < num_sensors; i++)  // loop over sensors
  {
      if(params.activeState[i] == HIGH)  // low means active
      {
        continue;  
      }

      if(params.switchMode[i])    // switch mode behaves differently
      {
        if(data[i] != -256)   {
              if(params.reverse[i]) {
                value = params.min_value[i] ;  
              } 
              else {
                value = params.max_value[i] ;  
              }             
        }
        else   {
              if(params.reverse[i]) {
                value = params.max_value[i] ;
              } 
              else {
                value = params.min_value[i] ;
              }             
        }
        
        if(value != prev_value[i]) {
              MIDI.sendControlChange(params.cc_num[i], value, 1);
              prev_value[i] = value;          
          }   
      }      
      else if(data[i] != -256)  // -256 is infinite distance from sensor (no object detected)
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

void sendRubberBand()
{
  for(int i=0; i < num_sensors; i++)  // loop over sensors
  {
      
      if(params.activeState[i] == HIGH)  // HIGH means OFF
      {
        continue;  
      }

      
      if(data[i] != -256)  // -256 is infinite distance from sensor (no hand detected)
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


bool buttonPress(int& reading, int& lastButtonState, int& state, int ledPin,
       int& buttonState,unsigned long& lastDebounceTime, unsigned long& debounceDelay)
{
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (reading != lastButtonState) {
    lastDebounceTime = millis();
  }

  if((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the button state has changed:
    if (reading != buttonState) {
      buttonState = reading;

      // only toggle the LED if the new button state is HIGH (press and release)
      if (buttonState == HIGH) {
        state = !state;
        // set the LED:
        digitalWrite(ledPin, state);
        lastButtonState = reading;
        return true;  // returns true if state has changed
      }
    }    
  }  

  // save the reading. Next time through the loop, it'll be the lastButtonState:
  lastButtonState = reading;
  return false;  
}

void ButtonHold(int buttonPin, int ledPin, unsigned long& holdStart, 
        unsigned long& holdTime, bool& holdPressed)
{
  // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
  if (!holdPressed && digitalRead(buttonPin) == LOW) {    
    holdStart = millis();
    holdPressed = true;
  }
  
  if((millis() - holdStart) > holdTime && holdPressed) {  // button has been held 
    holdMode(ledPin, buttonPin);  
  }

  
}

void holdMode(int ledPin, int buttonPin)
{
    // switch leds to active sensors!
    digitalWrite(ledPin3, params.activeState[0]);
    digitalWrite(ledPin4, params.activeState[1]);
    digitalWrite(ledPin5, params.activeState[2]);
    
    while(digitalRead(buttonPin) == LOW)
    {
      digitalWrite(ledPin, !holdMode);
      delay(1000);
      digitalWrite(ledPin, holdMode);    
      delay(1000);

      int r3 = digitalRead(buttonPin3);
      int r4 = digitalRead(buttonPin4);
      int r5 = digitalRead(buttonPin5); 
      HoldModeButtons(r3, r4, r5);  
      // switch leds to active sensors!
      digitalWrite(ledPin3, params.activeState[0]);
      digitalWrite(ledPin4, params.activeState[1]);
      digitalWrite(ledPin5, params.activeState[2]);
      
    }  
    holdPressed = false;
}

void initialise_preset1() {
    params.cc_num[0] = 22;  params.cc_num[1] = 23; params.cc_num[2] = 24;
    params.reverse[0] = false; params.reverse[1] = false; params.reverse[2] = false;
    params.switchMode[0] = false;  params.switchMode[1] = false;   params.switchMode[2] = false;
    params.activeState[0] = LOW; params.activeState[1] = LOW; params.activeState[2] = LOW;
    // (min, max) = (40, 150) sensor range due to terabee sensor characteristics
    params.sensor_range[0] = 150; params.sensor_range[1] = 150; params.sensor_range[2] = 150; 
    params.default_value[0] = 64; params.default_value[1] = 64; params.default_value[2] = 64;  
    // lowest possible is 0, highest possible is 127
    params.min_value[0] = 0; params.min_value[1] = 0; params.min_value[2] = 0;
    params.max_value[0] = 127; params.max_value[1] = 127; params.max_value[2] = 127;
    params.activePreset = 1;
    params.presetState[0] = LOW; params.presetState[1] = HIGH; params.presetState[2] = HIGH;
}

void initialise_preset2() {
    params.cc_num[0] = 22;  params.cc_num[1] = 23; params.cc_num[2] = 24;
    params.reverse[0] = true; params.reverse[1] = true; params.reverse[2] = true;
    params.switchMode[0] = true;  params.switchMode[1] = true;   params.switchMode[2] = false;
    params.activeState[0] = LOW; params.activeState[1] = LOW; params.activeState[2] = LOW;
    params.sensor_range[0] = 50; params.sensor_range[1] = 150; params.sensor_range[2] = 100; 
    params.default_value[0] = 30; params.default_value[1] = 90; params.default_value[2] = 30;  
    params.min_value[0] = 0; params.min_value[1] = 0; params.min_value[2] = 80;
    params.max_value[0] = 127; params.max_value[1] = 127; params.max_value[2] = 127;
    params.activePreset = 2;
    params.presetState[0] = HIGH; params.presetState[1] = LOW; params.presetState[2] = HIGH;
}

void initialise_preset3() {
    params.cc_num[0] = 26;  params.cc_num[1] = 24; params.cc_num[2] = 23;
    params.reverse[0] = false; params.reverse[1] = false; params.reverse[2] = false;
    params.switchMode[0] = false;  params.switchMode[1] = false;   params.switchMode[2] = false;
    params.activeState[0] = LOW; params.activeState[1] = LOW; params.activeState[2] = LOW;
    params.sensor_range[0] = 150; params.sensor_range[1] = 150; params.sensor_range[2] = 150; 
    params.default_value[0] = 64; params.default_value[1] = 64; params.default_value[2] = 64;  
    params.min_value[0] = 0; params.min_value[1] = 0; params.min_value[2] = 0;
    params.max_value[0] = 127; params.max_value[1] = 127; params.max_value[2] = 127;
    params.activePreset = 3;
    params.presetState[0] = HIGH; params.presetState[1] = HIGH; params.presetState[2] = LOW;
}

void Upload_preset_state()  {
  if(params.activePreset == 1)
  {
      initialise_preset1();
  }
  else if(params.activePreset == 2)
  {
      initialise_preset2();
  }
  else if(params.activePreset == 3)
  {
      initialise_preset3();
  }
}

// TO DEBUG
void HoldModeButtons(int r3, int r4, int r5)
{   
  // PRESET BUTTON 1
  buttonPress(r3, lastButtonState3, params.activeState[0], ledPin3,
                buttonState3, lastDebounceTime3, debounceDelay3);

  // PRESET BUTTON 2
  buttonPress(r4, lastButtonState4, params.activeState[1], ledPin4,
                buttonState4, lastDebounceTime4, debounceDelay4);

  // PRESET BUTTON 3
  buttonPress(r5, lastButtonState5, params.activeState[2], ledPin5,
                buttonState5, lastDebounceTime5, debounceDelay5);  
}
