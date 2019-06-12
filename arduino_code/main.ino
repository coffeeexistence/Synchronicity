#include "FastLED.h"
#include "serial_messaging.ino"

#define PIN 15
#define LED_BUILTIN 2

#define pixel_count 150

#define verboseSerialOutput false

// Extraneous duplication here
uint8_t colorPresetsLength = 6;
RGBStruct colorPresets[6] = {
    {0, 0, 0},   // 0 COLOR_BLANK
    {24, 8, 8},  // 1 COLOR_SOFT_RED
    {8, 24, 8},  // 2 COLOR_SOFT_GREEN
    {8, 8, 24},  // 3 COLOR_SOFT_BLUE
    {24, 8, 24}, // 4 COLOR_SOFT_?
    {24, 24, 8}, // 5 COLOR_SOFT_?
};

CRGB currentLEDState[pixel_count];
RGBStruct destinationState[pixel_count];

int loopDelay = 1;
int displayRate = 10; // Every n interpolations
uint16_t iterationCount = 0;

void setup()
{
  Serial.begin(115200); // opens serial port, sets data rate to 115200 bps
  FastLED.addLeds<WS2812, PIN>(currentLEDState, pixel_count);
  FastLED.show();
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  if (iterationCount == 1000)
  {
    iterationCount = 0;
  }

  serial_checkForNewStripInstruction();
  // serial_hasNewStripInstruction will be set to true if serial_checkForNewStripInstruction() gets a full instruction from the serial port
  if (serial_hasNewStripInstruction)
  {
    // Calling serial_getNewSerialInstruction() will set serial_hasNewStripInstruction back to false
    StripInstruction instruction = serial_getNewSerialInstruction();
    setDestinationStateFromStripInstruction(instruction);
  }

  if (iterationCount % displayRate == 0)
  {
    interpolateCurrentStateTowardsDestinationState();
    FastLED.show();
  }

  iterationCount++;
  delay(loopDelay);
}

void onUnexpectedData() { Serial.println("UNEX"); }

void setDestinationStateFromStripInstruction(StripInstruction instruction)
{
  if (instruction.startPixel >= pixel_count)
    return onUnexpectedData();
  if (instruction.endPixel >= pixel_count)
    return onUnexpectedData();
  if (instruction.colorPreset > colorPresetsLength)
    return onUnexpectedData();

  for (uint8_t i = instruction.startPixel; i <= instruction.endPixel; i++)
  {
    destinationState[i] = colorPresets[instruction.colorPreset];
  }
}

void interpolateCurrentStateTowardsDestinationState()
{
  for (uint8_t i = 0; i < pixel_count; i++)
  {
    currentLEDState[i].r = destinationState[i].r;
    currentLEDState[i].g = destinationState[i].g;
    currentLEDState[i].b = destinationState[i].b;
  }
}