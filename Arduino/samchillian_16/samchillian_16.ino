#include "Adafruit_WS2801.h"
#include "SPI.h" 


/*
 MIDI note player
 
 This sketch shows how to use the serial transmit pin (pin 1) to send MIDI note data.
 If this circuit is connected to a MIDI synth, it will play 
 the notes F#-0 (0x1E) to F#-5 (0x5A) in sequence.

 
 The circuit:
 * digital in 1 connected to MIDI jack pin 5
 * MIDI jack pin 2 connected to ground
 * MIDI jack pin 4 connected to +5V through 220-ohm resistor
 Attach a MIDI cable to the jack, then to a MIDI synth, and play music.

 created 13 Jun 2006
 modified 13 Aug 2012
 by Tom Igoe 

 This example code is in the public domain.
 
 http://www.arduino.cc/en/Tutorial/Midi
 
 */
 
/* #define MINMIDI 0
 #define MAXMIDI 127 */

#define DEBOUNCE 20
 #define FIRSTCLOCKNO  10 /*because joe cut out some lights i changed was this*/
 #define FIRSTBUTTNO  0
 #define NUMBUTTONS 9
 #define OCTINDICATORLIGHT 9 /*and this*/
 
 #define MIDIOUTBUFSIZE 32
#define KEYBUFSIZE 32

#define INCKEYBUFWRP ++keybufwrp; if (keybufwrp>=keybuf+KEYBUFSIZE) keybufwrp=keybuf;
#define INCKEYBUFRDP ++keybufrdp; if (keybufrdp>=keybuf+KEYBUFSIZE) keybufrdp=keybuf;

#define INCOUTMIDIBUFWRP ++outmidibufwrp; if (outmidibufwrp>=outmidibuf+MIDIOUTBUFSIZE) outmidibufwrp=outmidibuf;
#define INCOUTMIDIBUFRDP ++outmidibufrdp; if (outmidibufrdp>=outmidibuf+MIDIOUTBUFSIZE) outmidibufrdp=outmidibuf;

/*#define BOTTOMOCT 2
#define TOPOCT 9 */

#define NUMOCTAVES 8

#include <PinChangeInt.h>

uint8_t dataPin  = 2;    // Yellow wire on Adafruit Pixels
uint8_t clockPin = 3; 

Adafruit_WS2801 strip = Adafruit_WS2801(20, dataPin, clockPin);


short int outmidibuf[MIDIOUTBUFSIZE], *outmidibufwrp, *outmidibufrdp;
short int keybuf[KEYBUFSIZE], *keybufwrp, *keybufrdp ;

short int i=0;
short int midinote = 0x40;

short int thescale[7];
short int theoct, thepos, theoctpos, thelastoctpos, thelastclockpos;
short int movecolors[NUMBUTTONS][3];
short int clockpositions[NUMBUTTONS];
 
 short int programchangekeydepressed =0;

short int patchselector[9];

volatile uint16_t interruptCount=0;

int inputstates[10];

int midinotes[9];

  unsigned long last_interrupt_time=0;
  unsigned long interrupt_time;


void noteOn(int cmd, int pitch, int velocity) {
  Serial.write(cmd);
  Serial.write(pitch);
  Serial.write(velocity);
}

void init_lights() {
  int i=FIRSTBUTTNO, j;
  
  
 for (j=0;j<19;j++) {
    strip.setPixelColor(j,Color(0,0,0));
 }
 
 strip.show();
    /*
 strip.setPixelColor( i++, Color(0,0,255));
 strip.setPixelColor( i++, Color(0,255,0));
 strip.setPixelColor( i++, Color(255,0,0));
 strip.setPixelColor( i++, Color(0,255,255));
 strip.setPixelColor( i++, Color(255,255,255)); //middle =0
 strip.setPixelColor( i++, Color(0,255,255));
 strip.setPixelColor( i++, Color(255,0,0));
 strip.setPixelColor( i++, Color(0,255,0));
 strip.setPixelColor( i++, Color(0,0,255));*/
 
for (i=FIRSTBUTTNO;i<NUMBUTTONS+FIRSTBUTTNO;++i) {
  strip.setPixelColor (i, Color(movecolors[i][0], movecolors[i][1], movecolors[i][2]));
}

 strip.show();
}

void B4interrupt() {
   interrupt_time=millis();
  if (interrupt_time - last_interrupt_time > DEBOUNCE) {
    if (digitalRead(4)==LOW) *keybufwrp = 0;
    else *keybufwrp = 10;
    INCKEYBUFWRP;
  }
  last_interrupt_time=interrupt_time;  
}


void B5interrupt() {
interrupt_time=millis();
  if (interrupt_time - last_interrupt_time > DEBOUNCE) {
    if (digitalRead(5)==LOW) *keybufwrp = 1;
    else *keybufwrp = 11;
    INCKEYBUFWRP;
  }
  last_interrupt_time=interrupt_time;  
}

void B6interrupt() {
  interrupt_time=millis();
  if (interrupt_time - last_interrupt_time > DEBOUNCE) {
    if (digitalRead(6)==LOW) *keybufwrp = 2;
    else *keybufwrp = 12;
    INCKEYBUFWRP;
  }
  last_interrupt_time=interrupt_time;  
}

void B7interrupt() {
  interrupt_time=millis();
  if (interrupt_time - last_interrupt_time > DEBOUNCE) {
    if (digitalRead(7)==LOW) *keybufwrp = 3;
    else *keybufwrp = 13;
    INCKEYBUFWRP;
  }
  last_interrupt_time=interrupt_time;  
}

void B8interrupt() {
  interrupt_time=millis();
  if (interrupt_time - last_interrupt_time > DEBOUNCE) {
    if (digitalRead(8)==LOW) *keybufwrp = 4;
    else *keybufwrp = 14;
    INCKEYBUFWRP;
  }
  last_interrupt_time=interrupt_time;  
}

void B9interrupt() {
 interrupt_time=millis();
  if (interrupt_time - last_interrupt_time > DEBOUNCE) {
    if (digitalRead(9)==LOW) *keybufwrp = 5;
    else *keybufwrp = 15;
    INCKEYBUFWRP;
  }
  last_interrupt_time=interrupt_time;  
}
void B10interrupt() {
 interrupt_time=millis();
  if (interrupt_time - last_interrupt_time > DEBOUNCE) {
    if (digitalRead(10)==LOW) *keybufwrp = 6;
    else *keybufwrp = 16;
    INCKEYBUFWRP;
  }
  last_interrupt_time=interrupt_time;  
}
void B11interrupt() {
interrupt_time=millis();
  if (interrupt_time - last_interrupt_time > DEBOUNCE) {
    if (digitalRead(11)==LOW) *keybufwrp = 7;
    else *keybufwrp = 17;
    INCKEYBUFWRP;
  }
  last_interrupt_time=interrupt_time;  
}
void B12interrupt() {
 interrupt_time=millis();
  if (interrupt_time - last_interrupt_time > DEBOUNCE) {
    if (digitalRead(12)==LOW) *keybufwrp = 8;
    else *keybufwrp = 18;
    INCKEYBUFWRP;
  }
  last_interrupt_time=interrupt_time;  
}

void B13interrupt() {
 interrupt_time=millis();
  if (interrupt_time - last_interrupt_time > DEBOUNCE) {
    if (digitalRead(13)==LOW) *keybufwrp = 9;
    else *keybufwrp = 19;
    INCKEYBUFWRP;
  }
  last_interrupt_time=interrupt_time;  
}

void initmovecolorsarray() {
  
  movecolors[0][0]=0;
movecolors[0][1]=0;
movecolors[0][2]=255;

movecolors[1][0]=0;
movecolors[1][1]=255;
movecolors[1][2]=0;
    
movecolors[2][0]=255;
movecolors[2][10]=0;
movecolors[2][2]=0;

movecolors[3][0]=255;
movecolors[3][1]=0;
movecolors[3][2]=255;

movecolors[4][0]=255;//white = 0
movecolors[4][1]=255;
movecolors[4][2]=255;

movecolors[5][0]=255;
movecolors[5][1]=0;
movecolors[5][2]=255;

movecolors[6][0]=255;
movecolors[6][1]=0;
movecolors[6][2]=0;

movecolors[7][0]=0;
movecolors[7][1]=255;
movecolors[7][2]=0;

movecolors[8][0]=0;
movecolors[8][1]=0;
movecolors[8][2]=255;

}

  void setup() {
  //  Set MIDI baud rate:
    int i,j;
   

 
/*   for (i=0;i<128;++i) noteOn(0x90,i,0);*/
  
    
    thelastclockpos=0;
  
  for (i=0;i<NUMBUTTONS;++i)  clockpositions[i]=-1;
        
    initmovecolorsarray();
    /* strip.setPixelColor( i++, Color(0,0,255));
 strip.setPixelColor( i++, Color(0,255,0));
 strip.setPixelColor( i++, Color(255,0,0));
 strip.setPixelColor( i++, Color(0,255,255));
 strip.setPixelColor( i++, Color(255,255,255)); //middle =0
 strip.setPixelColor( i++, Color(0,255,255));
 strip.setPixelColor( i++, Color(255,0,0));
 strip.setPixelColor( i++, Color(0,255,0));
 strip.setPixelColor( i++, Color(0,0,255));*/


thescale[0]=0;
thescale[1]=2;
thescale[2]=4;
thescale[3]=5;
thescale[4]=7;
thescale[5]=9;
thescale[6]=11;

patchselector[0]=0; //piano
patchselector[1]=12; //marimba
patchselector[2]=19; //church org
patchselector[3]=23; //tango accordion
patchselector[4]=42; //cello
patchselector[5]=46; //orch harp
patchselector[6]=60; //french horn
patchselector[7]=70; //bassoon
patchselector[8]=90; //polysynth

for (i=0;i<9;++i) midinotes[i]=0;
for (i=0;i<10;++i) inputstates[i]=HIGH;


theoct = 4;
thepos = 0;

thelastoctpos=0;
theoctpos = 7*theoct+thepos;

 outmidibufrdp=outmidibuf;
 outmidibufwrp=outmidibuf;
 keybufwrp=keybuf;
 keybufrdp=keybuf;


   strip.begin();

  // Update LED contents, to start they are all 'off'
  strip.show();
  
  colorWipe(Color(255, 0, 0), 50);
 
    
    init_lights();    
  
    strip.show();
   
    pinMode(4,INPUT);
    pinMode(5, INPUT);
    pinMode(6, INPUT);
    pinMode(7, INPUT);
    pinMode(8, INPUT);
    pinMode(9, INPUT);
    pinMode(10, INPUT);
    pinMode(11, INPUT);
    pinMode(12, INPUT);
  pinMode(13,INPUT);
  
     Serial.begin(31250);//MIDI
   
/*Serial.begin(115200); //monitor print */

   for (i=0;i<128;++i) noteOn(0x90,i,0);
   
  /*attachPinChangeInterrupt(4, B4interrupt, CHANGE);
  attachPinChangeInterrupt(5, B5interrupt, CHANGE);
  attachPinChangeInterrupt(6, B6interrupt, CHANGE);
  attachPinChangeInterrupt(7, B7interrupt, CHANGE);
  attachPinChangeInterrupt(8, B8interrupt, CHANGE);
  attachPinChangeInterrupt(9, B9interrupt, CHANGE);
 attachPinChangeInterrupt(10, B10interrupt, CHANGE);
  attachPinChangeInterrupt(11, B11interrupt, CHANGE);
  attachPinChangeInterrupt(12, B12interrupt, CHANGE);*/
  


}

void displaynote(short int tm, short int tp) {
 short int ci = tm + 4;/*0-8: tm=themove, tp=thepos, ci=colorindex */
 short int clockpos = tp + FIRSTCLOCKNO;
 
/* Serial.println(clockpos);
 Serial.println(theOP);
 Serial.println(" ");*/
 
//if (thelastclockpos!=0) strip.setPixelColor(thelastclockpos, Color(0,0,0));
//thelastclockpos=clockpos;

 strip.setPixelColor( clockpos, Color(movecolors[ci][0],movecolors[ci][1], movecolors[ci][2]));
 
// strip.setPixelColor (OCTINDICATORLIGHT, Wheel(theoct*10+10 %255));
 
 //strip.setPixelColor (OCTINDICATORLIGHT, Wheel(84));
 
 strip.setPixelColor (OCTINDICATORLIGHT, Wheel((84*theoct+4)/8));
 
 
 strip.setPixelColor (ci+FIRSTBUTTNO, Color(0,0,0));
 

 strip.show();
  }

void loop() {
  int i;
  int newstate;
  
  for (i=0;i<10;++i)  {
    newstate = digitalRead(i+4);
    if (inputstates[i]!= newstate) {
      
      interrupt_time=millis();
  if (interrupt_time - last_interrupt_time > DEBOUNCE) {
 
       
      inputstates[i]=newstate;
      if (newstate==LOW) *keybufwrp = i;
          else *keybufwrp = 10+i;
      INCKEYBUFWRP;     
       
  last_interrupt_time=interrupt_time;  
}
    }
  }
  
  while (keybufrdp != keybufwrp) {
    

    if (*keybufrdp==9) programchangekeydepressed = 1;
    else if  (*keybufrdp==19) programchangekeydepressed = 0;
    
   //so we are no longer dealing with a depression or release of patch change button
   
   //let's first see if the button is down in which case we are dealing with patch change command
   
   else if (programchangekeydepressed == 1) {//select new sound 0-8
   
      if(*keybufrdp<10) { //ignore release of program change selection button
     
  *outmidibufwrp = 0xc0;
      INCOUTMIDIBUFWRP
      *outmidibufwrp = patchselector[*keybufrdp];
      INCOUTMIDIBUFWRP
   }
   }
    else if (*keybufrdp>=10) { //break code
      
      *outmidibufwrp = 0x90;
      INCOUTMIDIBUFWRP
      *outmidibufwrp = midinotes[*keybufrdp-10];
      INCOUTMIDIBUFWRP
       *outmidibufwrp = 0x00;
     INCOUTMIDIBUFWRP
       
    midinotes[*keybufrdp-10]=0;
    
 //   strip.setPixelColor( (thelastoctpos%7)+FIRSTCLOCKNO, Color(0,0,0));
    strip.setPixelColor( clockpositions[*keybufrdp-10]+FIRSTCLOCKNO, Color(0,0,0));
    int ci=*keybufrdp-10;
    strip.setPixelColor (ci+FIRSTBUTTNO, Color(movecolors[ci][0], movecolors[ci][1],movecolors[ci][2]));
 
    strip.show();
    
    
    clockpositions[*keybufrdp-10]=-1;
    }
    else {
  
      theoctpos = theoctpos + *keybufrdp-4;
  /*    if (theoctpos >= MAXMIDI || theoctpos <=MINMIDI) {
        theoctpos= theoctpos - *keybufrdp+4; 
      }*/
      
      if (theoctpos <0) theoctpos+=(NUMOCTAVES*7);
      if (theoctpos>NUMOCTAVES*7) theoctpos-=NUMOCTAVES*7;
      
      thepos = theoctpos % 7;
      theoct = ((theoctpos / 7) % NUMOCTAVES)+1;
      
      /* below used to be if (theoct>TOPOCT && thepos>0)*/
      
/*      if (theoct>TOPOCT) {
        theoct = BOTTOMOCT;
        theoctpos -= 7*(TOPOCT-BOTTOMOCT);
      }
      
      if (theoct<BOTTOMOCT) {
        theoct = TOPOCT;
        theoctpos += 7*(TOPOCT-BOTTOMOCT);
      }
      
     */
      midinote = theoct * 12 + thescale[thepos];
          clockpositions[*keybufrdp]=thepos;

    

     *outmidibufwrp = 0x90;
    INCOUTMIDIBUFWRP
      *outmidibufwrp = midinote;
     INCOUTMIDIBUFWRP
       *outmidibufwrp = 0x58;
     INCOUTMIDIBUFWRP
         displaynote(*keybufrdp-4, thepos);
      thelastoctpos=theoctpos;
        midinotes[*keybufrdp]=midinote;
    }
      
  INCKEYBUFRDP
  
  }
  
  
  while (outmidibufrdp != outmidibufwrp) {
 
 //  digitalWrite(13,HIGH);
 //  delay(1000);
   
 //  digitalWrite(13,LOW);
 
 
   Serial.write(*outmidibufrdp);
  INCOUTMIDIBUFRDP
  }

}




void rainbow(uint8_t wait) {
  int i, j;
   
  for (j=0; j < 256; j++) {     // 3 cycles of all 256 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel( (i + j) % 255));
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

// Slightly different, this one makes the rainbow wheel equally distributed 
// along the chain
void rainbowCycle(uint8_t wait) {
  int i, j;
  
  for (j=0; j < 256 * 5; j++) {     // 5 cycles of all 25 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      // tricky math! we use each pixel as a fraction of the full 96-color wheel
      // (thats the i / strip.numPixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 96 is to make the wheel cycle around
      strip.setPixelColor(i, Wheel( ((i * 256 / strip.numPixels()) + j) % 256) );
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

// fill the dots one after the other with said color
// good for testing purposes
void colorWipe(uint32_t c, uint8_t wait) {
  int i;
  
  for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

/* Helper functions */

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85) {
   return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
   WheelPos -= 85;
   return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170; 
   return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}
