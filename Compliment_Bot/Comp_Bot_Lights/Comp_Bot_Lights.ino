#include <FastLED.h>

#define LED_PIN         2
#define LED_WIDTH       16
#define LED_HEIGHT      16
#define NUM_LEDS        (LED_WIDTH * LED_HEIGHT)

#define MAX_BRIGHTNESS  100
#define FRAME_DELAY     10
#define FADE_AMOUNT     70

#define BOOTUP_DELAY    25
#define FADE_STEP       10
#define BRIGHTNESS_STEP 5
#define FADEIN_DELAY    30

#define AUDIO_PIN       A1
#define STABLE_THRESHOLD 10

CRGB leds[NUM_LEDS];
int audioHistory[3] = {0, 0, 0};

bool equalizerActive = false;
bool booting = false;
bool hasBootedSinceLastCleanSignal = false;
uint16_t equalizerFrame = 0;

int pulseLEDIndex = 0;
int pulseBrightness = 0;
bool pulseIncreasing = true;

// === Map (x, y) to LED index for serpentine layout ===
int getPixelIndex(int x, int y) {
  if (y % 2 == 0) return y * LED_WIDTH + x;
  else return y * LED_WIDTH + (LED_WIDTH - 1 - x);
}

// === Setup ===
void setup() {
  Serial.begin(9600);
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(MAX_BRIGHTNESS);
  pinMode(AUDIO_PIN, INPUT);

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  bootUpAnimation();  // Run once on power-up
}

void loop() {
  // Read and track audio
  int currentAudio = analogRead(AUDIO_PIN);
  Serial.println(currentAudio);
  audioHistory[0] = audioHistory[1];
  audioHistory[1] = audioHistory[2];
  audioHistory[2] = currentAudio;

  // Check for clean signal
  bool signalIsClean = false;
  if (audioHistory[0] > 0) {
    int minVal = min(audioHistory[0], min(audioHistory[1], audioHistory[2]));
    int maxVal = max(audioHistory[0], max(audioHistory[1], audioHistory[2]));
    signalIsClean = (maxVal - minVal <= STABLE_THRESHOLD);
  }

  // === Manage animation states ===
  if (signalIsClean) {
    if (!equalizerActive && !booting) {
      booting = true;
      hasBootedSinceLastCleanSignal = true;
      bootUpAnimation();
      equalizerActive = true;
      booting = false;
      equalizerFrame = 0;
    }
  } else {
    equalizerActive = false;
    hasBootedSinceLastCleanSignal = false;
  }

  if (equalizerActive && !booting) {
    drawEqualizerFrame(equalizerFrame);
    FastLED.show();
    delay(FRAME_DELAY);
    equalizerFrame += 2;
  } else {
    idlePulseAnimation();
    FastLED.show();
    delay(30);
  }
}

// === Boot-up Animation: sweep and fade ===
void bootUpAnimation() {
  for (int x = 0; x < LED_WIDTH; x++) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    for (int y = 0; y < LED_HEIGHT; y++) {
      leds[getPixelIndex(x, y)] = CRGB::White;
    }
    FastLED.show();
    delay(BOOTUP_DELAY);
  }

  for (int x = LED_WIDTH - 1; x >= 0; x--) {
    fill_solid(leds, NUM_LEDS, CRGB::Black);
    for (int y = 0; y < LED_HEIGHT; y++) {
      leds[getPixelIndex(x, y)] = CRGB::White;
    }
    FastLED.show();
    delay(BOOTUP_DELAY);
  }

  for (int fade = 255; fade >= 0; fade -= FADE_STEP) {
    for (int i = 0; i < NUM_LEDS; i++) {
      leds[i].nscale8_video(fade);
    }
    FastLED.show();
    delay(20);
  }

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

// === Equalizer Frame Animation ===
void drawEqualizerFrame(uint16_t t) {
  fadeToBlackBy(leds, NUM_LEDS, FADE_AMOUNT);

  for (int x = 0; x < LED_WIDTH; x++) {
    float wave = sin8((x * 16) + t) / 255.0;
    int height = map(wave * 100, 0, 100, 0, LED_HEIGHT / 2);
    int center = LED_HEIGHT / 2;

    for (int i = 0; i < height; i++) {
      int rowUp = center - i;
      int rowDown = center + i - 1 + (LED_HEIGHT % 2);

      if (rowUp >= 0) leds[getPixelIndex(x, rowUp)] = CHSV(x * 16, 255, 255);
      if (rowDown < LED_HEIGHT) leds[getPixelIndex(x, rowDown)] = CHSV(x * 16, 255, 255);
    }
  }
}

// === Idle Animation: fade random LED in center 64 ===
void idlePulseAnimation() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);

  // Animate current LED
  leds[pulseLEDIndex] = CRGB(pulseBrightness, pulseBrightness, pulseBrightness);

  // Update brightness
  if (pulseIncreasing) {
    pulseBrightness += 5;
    if (pulseBrightness >= 255) {
      pulseBrightness = 255;
      pulseIncreasing = false;
    }
  } else {
    pulseBrightness -= 5;
    if (pulseBrightness <= 0) {
      pulseBrightness = 0;
      pulseIncreasing = true;

      // Pick a new random LED in center 8x8 region
      int x = random(4, 12);  // center columns
      int y = random(4, 12);  // center rows
      pulseLEDIndex = getPixelIndex(x, y);
    }
  }
}
