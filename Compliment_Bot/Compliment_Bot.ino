#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// Use pins 2 and 3 to communicate with DFPlayer Mini
static const uint8_t PIN_MP3_TX = 2; // Connects to module's RX
static const uint8_t PIN_MP3_RX = 3; // Connects to module's TX
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);

DFRobotDFPlayerMini player; // Create the Player object

long randNumber; // Initialize your variable to randomize the track number
int loopCounter = 0; // debug value

int analogPin = A1; // microwave sensor relay lead 1 connected to analog pin 1
                    // microwave sensor relay lead 2 to 3V3 pin
int val = 0;  // variable to store the value read
int tempVal = 0; // temp variable for isValueClean function
int lowerVal = 0; // reference variable for isValueClean function
int upperVal = 0; // reference variable for isValueClean function

void setup() {
  randomSeed(analogRead(0)); //Use randomSeed() to start random function at different intervals
  Serial.begin(9600); // Init USB serial port for debugging
  softwareSerial.begin(9600); // Init serial port for DFPlayer Mini

  if (player.begin(softwareSerial)) { // Start communication with DFPlayer Mini
    Serial.println("*********************************************");
    Serial.println("*  Connection to DFPlayer Mini successful!  *");

    player.volume(30); // Set volume to maximum (0 to 30).
    player.play(1); //Play boot up mp3 file
    Serial.println("*  Welcome file played                      *");
    Serial.println("*********************************************");

  } else {
    Serial.println("*********************************************");
    Serial.println("*  Connecting to DFPlayer Mini failed!      *");
    Serial.println("*********************************************");
  }
    delay(5000);
}

// Boolean function to see if the microwave sensor values are staying consistent
// If they are consistent within 5 units of eachother we know the microwave sensor relay is open and function returns true (Clean)
// Else they are not consistent within 5 units of eachother the microwave sensor relay is closed and function returns false (Dirty)
bool isValueClean() { 
  tempVal = val; // Update tempVal with value of most recent microwave sensor relay reading [initialized in loop function]
  Serial.print("tempVal is: ");
  Serial.println(tempVal);
  val = analogRead(analogPin); // Update val with value from current microwave sensor relay reading
  Serial.print("val is: ");
  Serial.println(val);
  lowerVal = tempVal - 5; // Set lower limit to reference val with (within 5 units lower)
  Serial.print("lowerVal is: ");
  Serial.println(lowerVal);
  upperVal = tempVal + 5; // Set upper limit to reference val with (within 5 units higher)
  Serial.print("upperVal is: ");
  Serial.println(upperVal);

  if(val > lowerVal && val < upperVal){ // Evaluate if val is within acceptable bounds (Clean or Dirty)
    Serial.println("Number is clean");
    return true;
  } else {
    Serial.println("Number is dirty");
    return false;
  }
}

void loop() {
  Serial.println("--------------------------------------------------");
  Serial.print("Loop Count: ");
  Serial.println(loopCounter); // debug value
  loopCounter = loopCounter + 1;

  randNumber = random(1,10); //set randNumber to be a random number between min-max (min, max)
  Serial.print("randNumber is: ");
  Serial.println(randNumber); // debug value
 
  val = analogRead(analogPin);

  if(isValueClean() == true) { //microwave sensor is giving values 668-670 for active relay

    player.play(randNumber); //Play track using randNumber track #
    Serial.print("track ");
    Serial.print(randNumber);
    Serial.println(" played!");
  }
  delay(5000);
}
