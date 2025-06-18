// - Compliment Bot Code - // - This code is made in part with ChatGPT

#include <FastLED.h>

// === LED Panel Configuration ===
#define LED_PIN         2           // Pin connected to the LED data line
#define LED_WIDTH       16          // Width of the matrix
#define LED_HEIGHT      16          // Height of the matrix
#define NUM_LEDS        (LED_WIDTH * LED_HEIGHT)  // Total number of LEDs

// === LED Brightness and Animation Timing ===
#define MAX_BRIGHTNESS  100         // Max brightness (0–255)
#define FRAME_DELAY     10          // Delay between frames of animations (ms)
#define FADE_AMOUNT     70          // Equalizer: trail decay intensity (higher = shorter trails)

// === Boot Animation Timing ===
#define BOOTUP_DELAY    20          // Delay between boot-up column frames (ms)
#define FADE_STEP       10          // Fade steps for boot-up fade-out
#define BRIGHTNESS_STEP 5           // Equalizer fade in/out step size
#define FADEIN_DELAY    5           // Delay between fade in/out steps (ms)

// === DFPlayer Mini Playback Detection ===
#define BUSY_PIN             3      // DFPlayer BUSY pin (LOW = playing, HIGH = stopped)
#define BUSY_RECHECK_DELAY   4000   // Wait time to confirm playback ended (non-blocking, ms)

CRGB leds[NUM_LEDS];

// === Animation States ===
bool equalizerActive = false;
bool booting = false;
uint16_t equalizerFrame = 0;

// === Idle Pulse LED State ===
int pulseLEDIndex = 0;
int pulseBrightness = 0;
bool pulseIncreasing = true;

// === BUSY Pin State Tracking ===
bool previousBusyState = HIGH;
unsigned long busyWentHighTime = 0;
bool waitingForFadeOut = false;

// === Map (x, y) to LED index for serpentine layout ===
int getPixelIndex(int x, int y) {
  if (y % 2 == 0) return y * LED_WIDTH + x;
  else return y * LED_WIDTH + (LED_WIDTH - 1 - x);
}

void setup() {
  // Serial.begin(9600); // Uncomment if debugging
  FastLED.addLeds<NEOPIXEL, LED_PIN>(leds, NUM_LEDS);
  FastLED.setBrightness(MAX_BRIGHTNESS);

  pinMode(BUSY_PIN, INPUT);
  digitalWrite(BUSY_PIN, HIGH);  // Optional pull-up (if not already pulled high)

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();

  bootUpAnimation();  // Initial visual startup sequence
}

void loop() {
  bool currentBusyState = (digitalRead(BUSY_PIN) == LOW);
  // Serial.println(currentBusyState ? "BUSY: LOW (Playing)" : "BUSY: HIGH (Stopped)");

  // === Playback Started ===
  if (currentBusyState && !previousBusyState) {
    if (!equalizerActive && !booting) {
      booting = true;
      bootUpAnimation();
      fadeInEqualizer();
      equalizerActive = true;
      booting = false;
      equalizerFrame = 0;
    }
    waitingForFadeOut = false;
  }

  // === Playback Ended — begin non-blocking recheck ===
  if (!currentBusyState && previousBusyState) {
    busyWentHighTime = millis();
    waitingForFadeOut = true;
  }

  // === Confirm stop after BUSY_RECHECK_DELAY ===
  if (waitingForFadeOut && millis() - busyWentHighTime >= BUSY_RECHECK_DELAY) {
    if (digitalRead(BUSY_PIN) == HIGH && equalizerActive && !booting) {
      fadeOutEqualizer();
      equalizerActive = false;
    }
    waitingForFadeOut = false;
  }

  // === Store current state for next loop ===
  previousBusyState = currentBusyState;

  // === Run active animation ===
  if (equalizerActive && !booting) {
    drawEqualizerFrame(equalizerFrame);
    FastLED.show();
    delay(FRAME_DELAY);
    equalizerFrame += 2;
  } else {
    idlePulseAnimation();
    FastLED.show();
    delay(FRAME_DELAY);
  }
}

// === Boot-up Animation: white column sweeps left and right ===
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
    delay(BOOTUP_DELAY);
  }

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.show();
}

// === Fade in Equalizer Animation ===
void fadeInEqualizer() {
  uint16_t t = 0;
  for (int b = 0; b <= MAX_BRIGHTNESS; b += BRIGHTNESS_STEP) {
    FastLED.setBrightness(b);
    drawEqualizerFrame(t);
    FastLED.show();
    delay(FADEIN_DELAY);
    t += 2;
  }
}

// === Fade out Equalizer Animation ===
void fadeOutEqualizer() {
  for (int b = MAX_BRIGHTNESS; b >= 0; b -= BRIGHTNESS_STEP) {
    FastLED.setBrightness(b);
    drawEqualizerFrame(0);
    FastLED.show();
    delay(FADEIN_DELAY);
  }

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  FastLED.setBrightness(MAX_BRIGHTNESS);
  FastLED.show();
}

// === Equalizer Pattern Animation ===
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

// === Idle Animation: Pulse one random LED in center area ===
void idlePulseAnimation() {
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

      // Pick a new LED in center 8×8
      int x = random(4, 12);
      int y = random(4, 12);
      pulseLEDIndex = getPixelIndex(x, y);
    }
  }

  fill_solid(leds, NUM_LEDS, CRGB::Black);
  leds[pulseLEDIndex] = CRGB(pulseBrightness, pulseBrightness, pulseBrightness);
}
