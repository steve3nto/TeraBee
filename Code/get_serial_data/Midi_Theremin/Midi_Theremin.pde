import processing.serial.*;
import java.util.Random;
import oscP5.*;
import netP5.*;

OscP5 oscP5;
NetAddress myRemoteLocation;

Serial myPort;  // The serial port
int dim = width;
int w_canvas = 1200;
int h_canvas = 800;

void setup() {
  // Serial data setup
  // List all the available serial ports
  printArray(Serial.list());
  // Open the port you are using at the rate you want:
  myPort = new Serial(this, Serial.list()[0], 115200);
  
  // Setup of OSC messaging
  /* start oscP5, listening for incoming messages at port 12000 */
  oscP5 = new OscP5(this,12000);
  myRemoteLocation = new NetAddress("127.0.0.1",57121);
  
  // Visualization Setup
  size(1200, 800);
  background(0);
  colorMode(HSB, 360, 100, 100);
  noStroke();
  ellipseMode(RADIUS);
  frameRate(125);
}

void draw() {
  while (myPort.available() > 0) {
    String inBuffer = myPort.readString();
    inBuffer = inBuffer.substring(2);
    String[] parts = inBuffer.split("\t");
    
    //int foo = Integer.parseInt(parts[0]);
    
    int S1, S2, S3, S4, S5;
    
    S1 = Integer.parseInt(parts[1]);
    S2 = Integer.parseInt(parts[2]);
    S3 = Integer.parseInt(parts[3]);
    S4 = Integer.parseInt(parts[4]);
    S5 = Integer.parseInt(parts[5]);
    
    //println(S1, S2, S3, S4, S5);
    int[] sensor = new int[]{S1, S2, S3, S4, S5};
      
    // define random position generator
    Random rand = new Random();
       
    // Draw something
    background(0);
    for (int i = 0; i < 5; i++) {
      int x = rand.nextInt(w_canvas - 30) + 30;
      int y = rand.nextInt(h_canvas - 30) + 30;
      drawGradient(x, y, sensor[i]);
    } 
    
    // Map sensor values to MIDI notes!
    int[] midi = new int[5];
    for(int i = 0; i < 5; i++)
    {
      //midi[i] = (sensor[i]-40)*(8-1)/(400-40) + 1;
      if (sensor[0] < 30) {
            midi[i] = 1;
        } else if (sensor[0] >= 30 && sensor[0] < 90) {
            midi[i] = 60;
        } else if (sensor[0] >= 90 && sensor[0] < 140) {
            midi[i] = 62;
        } else if (sensor[0] >= 140 && sensor[0] < 180) {
            midi[i] = 64;
        } else if (sensor[0] >= 180 && sensor[0] < 220) {
            midi[i] = 65;
        } else if (sensor[0] >= 220 && sensor[0] < 275) {
            midi[i] = 67;
        } else if (sensor[0] >= 275 && sensor[0] < 323) {
            midi[i] = 69;    
        } else {
            midi[i] = 71;
        }
      
    }
        
    OscMessage myMessage = new OscMessage("/SC/inputs");
    //myMessage.add(mouseX/(float)width);
    //myMessage.add(mouseY/(float)height);
    myMessage.add((float)midi[0]);
    myMessage.add((float)midi[1]);
    myMessage.add((float)midi[2]);
    myMessage.add((float)midi[3]);
    myMessage.add((float)midi[4]);
    oscP5.send(myMessage, myRemoteLocation); 
    myMessage.print();
  }
}

void drawGradient(float x, float y, int h) {
  int radius = dim/2;

  for (int r = radius; r > 0; --r) {
    fill(h, 90, 90);
    ellipse(x, y, r, r);
    h = (h + 1) % 360;
  }
}
