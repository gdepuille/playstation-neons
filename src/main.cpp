#include <Arduino.h>
#include <FastLED.h>
#include <OneButton.h>

//#define DEBUG

// Input pin for the button
// Click : Change manuelly the pattern
// Double click : Enable / Disable automatic change
// Long press : Change the brightness
#define BTN 2

// Triangle 38 ; Rond 35 ; Croix 31 ; Carré 41
#define TRIANGLE 38
#define ROND 35
#define CROIX 31
#define CARRE 41
#define NUM_LEDS (TRIANGLE + ROND + CROIX + CARRE)

#define FRAMES_PER_SECOND 120
#define TIME_TO_SLEEP_MINUTE 60 // 1 Heure

#define ARRAY_SIZE(A) (sizeof(A) / sizeof((A)[0]))

void addGlitter(fract8 v);
CRGB symbolColor(uint8_t led);

void playStationAnim();
void playStationFixe();
void rainbow();
void rainbowWithGlitter();
void confetti();
void sinelon();
void juggle();
void bpm();

void nextPattern();
void toggleAuto();
void multiClick();
void startChangeBrightness();
void stopChangeBrightness();
void modifyBrightness();

// List of patterns to cycle through.  Each is defined as a separate function below.
typedef void (*SimplePatternList[])();
SimplePatternList gPatterns = {playStationAnim, playStationFixe, rainbow, rainbowWithGlitter, confetti, sinelon, juggle, bpm};

uint8_t gCurrentPatternNumber = 0; // Index number of which pattern is current
uint8_t gHue = 0;                  // rotating "base color" used by many of the patterns


bool modeChanged = false;
bool automaticChange = true;
bool changeBrightness = false;
bool incBrightness = true;
bool enabled = true;
uint8_t brightness = 50;
uint8_t elapsedRuntime = 0;
CRGB leds[NUM_LEDS];

OneButton button = OneButton(BTN, true, true); // Low level + internal pullup

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println("Setup");

  Serial.println("Config LED BUILTIN");
#endif
  pinMode(LED_BUILTIN, OUTPUT);

#ifdef DEBUG
  Serial.println("Configuration bouton");
#endif
  button.attachClick(nextPattern);
  button.attachDoubleClick(toggleAuto);
  button.attachMultiClick(multiClick);
  button.attachLongPressStart(startChangeBrightness);
  button.attachLongPressStop(stopChangeBrightness);

#ifdef DEBUG
  Serial.println("Configuration bandeau LEDs");
#endif
  FastLED.addLeds<NEOPIXEL, 6>(leds, NUM_LEDS);
  // set master brightness control
  FastLED.setBrightness(brightness);
}

void loop()
{
  button.tick();

  unsigned long delay = 1000 / FRAMES_PER_SECOND;
  if (enabled && modeChanged) {
    for (int i = 0 ; i < NUM_LEDS ; i++) {
      if (automaticChange) {
        leds[i] = CRGB(0, 255, 0);
      } else {
        leds[i] = CRGB(255, 0, 0);
      }
    }
    modeChanged = false;
    delay = 5000;
  
  } else if (enabled && !modeChanged) {
    // Call the current pattern function once, updating the 'leds' array
    gPatterns[gCurrentPatternNumber]();
  
  } else if (!enabled) {
    fadeToBlackBy(leds, NUM_LEDS, 20);
    elapsedRuntime = 0;
  }

  FastLED.show(); // send the 'leds' array out to the actual LED strip
  FastLED.delay(delay); // insert a delay to keep the framerate modest
  
  // do some periodic updates
  EVERY_N_MILLISECONDS(20) { 
     // slowly cycle the "base color" through the rainbow
    gHue++; 
  }

  EVERY_N_MILLISECONDS(100) {
    // change the brightness
    if (changeBrightness) {
      modifyBrightness();
    }
  }

  EVERY_N_MINUTES(1) {
    // increment time
    if (enabled) {
      elapsedRuntime++;
    }

#ifdef DEBUG
    Serial.println("Auto stop : ");
    Serial.print(" -> Elapsed (min)   : ");Serial.println(elapsedRuntime);
    Serial.print(" -> Stop time (min) : ");Serial.println(TIME_TO_SLEEP_MINUTE);
#endif

    if (elapsedRuntime >= TIME_TO_SLEEP_MINUTE) {
      enabled = false;
    }
  }

  EVERY_N_SECONDS(30) { 
    // change patterns periodically if in automatic mode
    if (enabled && automaticChange) {
      nextPattern(); 
    }
  } 
}

// -------------------------------------------------- //
void nextPattern() {
  if (!enabled) {
    enabled = true;
    return;
  }

#ifdef DEBUG
  Serial.println("Click -> Pattern change");
#endif

  // add one to the current pattern number, and wrap around at the end
  gCurrentPatternNumber = (gCurrentPatternNumber + 1) % ARRAY_SIZE(gPatterns);

#ifdef DEBUG
  Serial.print(" * Pattern courant : ");
  Serial.println(gCurrentPatternNumber);
#endif  
}

void toggleAuto() {
  if (!enabled) {
    enabled = true;
    return;
  }

#ifdef DEBUG
  Serial.println("Doubleclick -> Pattern automatic change");
#endif
  automaticChange = !automaticChange;
  modeChanged = true;

#ifdef DEBUG
  Serial.print(" * Enchainement en mode auto : ");
  Serial.println(automaticChange);
#endif
}

void multiClick() {
  if (!enabled) {
    enabled = true;
    return;
  }

  int nbClick = button.getNumberClicks();
#ifdef DEBUG
  Serial.print("Multiclick -> Pattern multiclick = ");
  Serial.println(nbClick);
#endif

  if (nbClick == 3) {
    enabled = false;
#ifdef DEBUG
  Serial.println(" * Extinction");
#endif
  } else {
#ifdef DEBUG
  Serial.println(" * Inconnu !!");
#endif
  }
}

void startChangeBrightness() {
  if (!enabled) {
    return;
  }

#ifdef DEBUG
  Serial.println("Longpress start -> Brightness change");
#endif

  changeBrightness = true;
}

void stopChangeBrightness() {
  if (!enabled) {
    return;
  }
#ifdef DEBUG
  Serial.println("Longpress stop -> Brightness change");
#endif

  changeBrightness = false;
  incBrightness = !incBrightness;
}

void modifyBrightness() {
  if (!enabled) {
    return;
  }

  if (incBrightness && brightness <= 250) {
    brightness += 5;
  } else if (!incBrightness && brightness >= 10) {
    brightness -= 5;
  }

  FastLED.setBrightness(brightness);

#ifdef DEBUG
  Serial.print(" * Brightness : ");
  Serial.println(brightness);
#endif
}

// -------------------------------------------------- //

CRGB symbolColor(uint8_t led)
{
  if (led < TRIANGLE)
  {
    return CRGB(0, 255, 0);
  }
  else if (led < TRIANGLE + ROND)
  {
    return CRGB(255, 0, 0);
  }
  else if (led < TRIANGLE + ROND + CROIX)
  {
    return CRGB(0, 0, 255);
  }
  else
  {
    return CRGB(253, 108, 158);
  }
}

void playStationAnim()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS - 1, 1000, 0);
  leds[pos] += symbolColor(pos);
}

void playStationFixe()
{
  for (int i = 0; i < NUM_LEDS; i++)
  {
    leds[i] = symbolColor(i);
  }
}

void rainbow()
{
  // FastLED's built-in rainbow generator
  fill_rainbow(leds, NUM_LEDS, gHue, 7);
}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter(fract8 chanceOfGlitter)
{
  if (random8() < chanceOfGlitter)
  {
    leds[random16(NUM_LEDS)] += CRGB::White;
  }
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy(leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV(gHue + random8(64), 200, 255);
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy(leds, NUM_LEDS, 20);
  int pos = beatsin16(13, 0, NUM_LEDS - 1);
  leds[pos] += CHSV(gHue, 255, 192);
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 62;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8(BeatsPerMinute, 64, 255);
  for (int i = 0; i < NUM_LEDS; i++)
  { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
}

void juggle()
{
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy(leds, NUM_LEDS, 20);
  byte dothue = 0;
  for (int i = 0; i < 8; i++)
  {
    leds[beatsin16(i + 7, 0, NUM_LEDS - 1)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
}
