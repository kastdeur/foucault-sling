/*
Foucault Sling (IR Trigger + NeoPixel Leds)
*/

#include <FastLED.h>

#define ENABLE_SERIAL 0
#define TRIG_LED 0

#if ESP32
#include <Arduino.h>
#define ENABLE_SERIAL 0
const int irPin = 14;
const int ledPin = 16;
#else /* not ESP32 */
const int irPin = 2;
const int ledPin = 7;
#endif /* ESP32 */

#define NUM_LEDS 12

const unsigned long irTriggerMinPeriod = 10UL; // milliseconds
const unsigned long irTriggerMaxPeriod = 1500UL; // milliseconds

const unsigned long irTriggerShortInterval = 1*1000UL;// 50;// milliseconds, between up and down swing
const unsigned long irTriggerRepeatInterval = 5*1000UL; // 7200;// milliseconds
const unsigned long irTriggerIntervalLeeway = 2000UL; // 200;// milliseconds

const unsigned long irTriggerSetStaleInterval = 120*1000UL; // 12 hours

const unsigned int irTriggerSetCountRequired = 3; // required consecutive triggers
const unsigned int irTriggerSetInvalidateCount = 3;

const unsigned long lampTimeSetInterval = 10*1000UL; // 10 seconds
const unsigned long lampsUpdateInterval = 1*1000UL;
const unsigned long heartbeatInterval = 1000UL;
const unsigned long timePerLed = 1*60*1000UL;

volatile int irState =  HIGH; // It is pulled low on trigger
volatile unsigned long irMills = 0; // Trigger time

unsigned long lampTime = 0; // zero when sling is overhead
unsigned long heartbeatMills = 0;
unsigned long curMills = 0;
unsigned long lampTimeSetMills = 0; // only set the lampTime once every lampUpdateInterval
unsigned long lampsUpdateMills = 0;

// timestamps for triggering
unsigned long irCurrentTriggerStart = 0;
unsigned long irCurrentTriggerEnd = 0;
unsigned long irPrevTriggerStart = 0;
unsigned long irTriggerSetStart = 0;
unsigned long irTriggerSetLast = 0;
int irTriggerSetCount = 0;
int irTriggerSetCountFinal = 0;

CRGB leds[NUM_LEDS];

#if ESP32
CRGB heartbeatLed[1];
const int heartbeatPin = 18;
#endif /* ESP32 */

void setup() {
  pinMode(irPin, INPUT);
  FastLED.addLeds<NEOPIXEL, ledPin>(leds, NUM_LEDS);

  #if ESP32
    FastLED.addLeds<NEOPIXEL, heartbeatPin>(heartbeatLed, 1);
  #endif /* ESP32 */

  // attach interrupt
  #if ESP32
    attachInterrupt(irPin, ir_interrupt, CHANGE);
  #else
    attachInterrupt(digitalPinToInterrupt(irPin), ir_interrupt, CHANGE);
  #endif /* ESP32 */

  #if ENABLE_SERIAL
  // Start Serial for debugging
  delay(1000);
  Serial.begin(9600);
  curMills = millis();
  while(!Serial || millis() - curMills < 5000) {;}
  Serial.println("Initialising");
  delay(1000);
  #endif /* ENABLE_SERIAL */
}

void loop() {
  // new loop
  curMills = millis();

  handle_ir_trigger();

  check_ir_trigger();

  update_lamptime();

  update_lamps();

  heartbeat();
}

void heartbeat() {
  if ( curMills - heartbeatMills < heartbeatInterval )
  { // wait a little longer
    return;
  }
  heartbeatMills = curMills;

  bool heartbeat = (curMills / heartbeatInterval) % 2;

  #if ESP32
  heartbeatLed[0] = heartbeat ? CRGB::Red : CRGB::Green;
  #else
  leds[0] =  heartbeat ? CRGB::Red : CRGB::Green;
  #endif /* ESP32 */

  FastLED.show();
}

void update_lamptime() {
  if ( irTriggerSetCountFinal < irTriggerSetCountRequired
      && irTriggerSetCount < irTriggerSetCountRequired )
  { // not yet enough counts
    return;
  }

  if ( curMills - lampTimeSetMills < lampTimeSetInterval )
  { // wait a little longer
    return;
  }

  int thisCount = irTriggerSetCountFinal;
  // if the current set is large enough use it
  if ( irTriggerSetCount >= irTriggerSetCountRequired )
  {
    thisCount = irTriggerSetCount;
  }

  lampTime = irTriggerSetStart + irTriggerRepeatInterval * thisCount / 2;

  lampTimeSetMills = curMills;
  #if ENABLE_SERIAL
  Serial.print("Setting lampTime: ");
  Serial.println(lampTime);
  #endif /* ENABLE_SERIAL */
}

void update_lamps() {
  #if ENABLE_TRIGLED
  // Debugging
  if ( irCurrentTriggerStart > irCurrentTriggerEnd ) {
    leds[6] = CRGB::Blue;
  }
  else {
    leds[6] = CRGB::Black;
  }
  FastLED.show();
  #endif /* ENABLE_TRIGLED */

  if ( curMills - lampsUpdateMills < lampsUpdateInterval )
  { // wait a little longer
    return;
  }

  if ( lampTime == 0 )
  { // no lamps yet
    return;
  }

  const bool backwards = false;
  const int trailLength = 3;

  int currentLed = ( (curMills - lampTime) / timePerLed) % NUM_LEDS;

    #if ENABLE_SERIAL
    if ( (curMills / lampsUpdateInterval) % 5 == 0 )
    {
      Serial.print("CurrentLed: ");
      Serial.print(currentLed);
      Serial.print(" (curMills - lampTime: ");
      Serial.print(curMills - lampTime);
      Serial.print(") / (timePerLed: ");
      Serial.print(timePerLed);
      Serial.print(") = ");
      Serial.println( (curMills - lampTime) / timePerLed );
    }
    #endif /* ENABLE_SERIAL */

  for ( int i = 0; i < NUM_LEDS; ++i)
  {
    int ledIdx = 0;
    if (backwards)
    {
      ledIdx = (i - currentLed + NUM_LEDS) % NUM_LEDS;
    }
    else
    {
      ledIdx = (currentLed - i + NUM_LEDS) % NUM_LEDS;
    }

    // set lights
    if (i == 0)
    {
      leds[ledIdx] = CRGB::White;
    }
    else if ( i < trailLength)
    {
      leds[ledIdx] = CRGB::Blue;
    }
    else
    {
      leds[ledIdx] = CRGB::Black;
    }
  }

  FastLED.show();
  lampsUpdateMills = curMills;
}
