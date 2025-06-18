// - Compliment Bot Code - // - This code is made in part with ChatGPT

#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"

// How many tracks are on the SD card?
const int trackAmount = 73;

// DFPlayer Mini serial pins
const uint8_t PIN_MP3_TX = 2;
const uint8_t PIN_MP3_RX = 3;
SoftwareSerial softwareSerial(PIN_MP3_RX, PIN_MP3_TX);
DFRobotDFPlayerMini player;

// Pin assignments
const int TRIGGER_OUTPUT_PIN = 4; // Outputs constant HIGH
const int TRIGGER_INPUT_PIN  = 5; // Reads HIGH when loop is closed
const int BUSY_PIN           = 6; // LOW = playing, HIGH = idle

// Debounce settings - This helps clean up and verify the signal thru the relay
const unsigned long DEBOUNCE_DELAY = 100;
unsigned long triggerHighStartTime = 0;
bool triggerConfirmed = false;

// Playback flags
bool track1Played = false;
bool randomTrackPlayed = false;

void setup() {
  randomSeed(analogRead(0));
  Serial.begin(9600);
  softwareSerial.begin(9600);

  pinMode(TRIGGER_OUTPUT_PIN, OUTPUT);
  digitalWrite(TRIGGER_OUTPUT_PIN, HIGH); // Always HIGH

  pinMode(TRIGGER_INPUT_PIN, INPUT); // Reads relay input
  pinMode(BUSY_PIN, INPUT);          // LOW = playing

  if (player.begin(softwareSerial)) {
    Serial.println("DFPlayer Mini initialized.");
    player.volume(30);
    player.play(1); // plays a little bootup animation
    delay(50); // gives the player a little time to trigger the audio
  } else {
    Serial.println("DFPlayer Mini connection failed.");
  }
}

void loop() {
  int triggerState = digitalRead(TRIGGER_INPUT_PIN);
  int busyState = digitalRead(BUSY_PIN); // LOW = playing
  unsigned long currentTime = millis();

  // === Debounce Trigger Input ===
  if (triggerState == HIGH) {
    if (triggerHighStartTime == 0) {
      triggerHighStartTime = currentTime;
    } else if ((currentTime - triggerHighStartTime) >= DEBOUNCE_DELAY && !triggerConfirmed) {
      triggerConfirmed = true;
      Serial.println("Trigger confirmed HIGH after debounce.");
    }
  } else {
    triggerHighStartTime = 0;
    triggerConfirmed = false;
    track1Played = false;
    randomTrackPlayed = false;
  }

  // === Debug output ===
  //Serial.print("Trigger: ");
  //Serial.print(triggerState);
  //Serial.print(" | BUSY: ");
  //Serial.print(busyState);
  //Serial.print(" | Confirmed: ");
  //Serial.println(triggerConfirmed);

  // === Play Track 1 if trigger confirmed and player is idle ===
  if (triggerConfirmed && busyState == HIGH && !track1Played) {
    Serial.println("Playing track 1...");
    player.play(1);
    track1Played = true;
    randomTrackPlayed = false;
    delay(50); // this was nessecary, as track 1 wasn't starting fast enough for the second if to trigger properly 
  }

  // === When track 1 finishes, play random track ===
  if (track1Played && !randomTrackPlayed && digitalRead(BUSY_PIN) == HIGH) {
    int randTrack = random(2, trackAmount);
    Serial.print("Playing random track: ");
    Serial.println(randTrack);
    player.play(randTrack);
    randomTrackPlayed = true;
  }
  delay(50);
}
