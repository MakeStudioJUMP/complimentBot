#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// Use pins 2 and 3 to communicate with DFPlayer Mini
static const uint8_t PIN_MP3_TX = 2; // Connects to module's RX
static const uint8_t PIN_MP3_RX = 3; // Connects to module's TX
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

DFRobotDFPlayerMini player; // Create the Player object

long randNumber; // Initialize your variable to randomize the track number

int analogPin = A1; // microwave sensor relay lead 1 connected to analog pin 1
                    // microwave sensor relay lead 2 to 3V3 pin
int val = 0;  // variable to store the value read

void setup() {
  randomSeed(analogRead(0)); //Use randomSeed() to start random function at different intervals
  Serial.begin(9600); // Init USB serial port for debugging
  softwareSerial.begin(9600); // Init serial port for DFPlayer Mini

  if (player.begin(softwareSerial)) { // Start communication with DFPlayer Mini
    Serial.println("Connection to DFPlayer Mini successful!");

    player.volume(30); // Set volume to maximum (0 to 30).

  } else {
    Serial.println("Connecting to DFPlayer Mini failed!");
  }
    delay(5000);
}

void loop() {
  val = analogRead(analogPin);  // read the input pin
  Serial.println("val is:");
  Serial.println(val);          // debug value
  
  randNumber = random(1,10); //set randNumber to be a random number between min-max (min, max)
  Serial.println("randNumber is:");
  Serial.println(randNumber); // debug value
 
  if(val > 700 && val < 720) { //microwave sensor is giving values 712-716 for active relay

    player.play(randNumber); //Play track using randNumber track #
    Serial.println("track played");
  }
  delay(5000);
}
