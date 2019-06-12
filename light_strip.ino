#include "FastLED.h"
#include "./type_definitions.h"
#include "./serial_messaging.h"
#include "./easing.h"
#include "LinkedList.h"

#define PIN 15
#define LED_BUILTIN 2
#define defaultAnimationDurationMillis 150
#define pixel_count 150

#define verboseSerialOutput false

#define COLOR_BLANK 0
uint8_t colorPresetsLength = 6;
CRGB colorPresets[6] = {
    {0, 0, 0},   // 0 COLOR_BLANK
    {24, 8, 8},  // 1 COLOR_SOFT_RED
    {8, 24, 8},  // 2 COLOR_SOFT_GREEN
    {8, 8, 24},  // 3 COLOR_SOFT_BLUE
    {24, 8, 24}, // 4 COLOR_SOFT_?
    {24, 24, 8}, // 5 COLOR_SOFT_?
};

CRGB currentLEDState[pixel_count];
LinkedList<StripAnimation> stripAnimationLinkedList;

int loopDelay = 1;
int displayRate = 5; // Every n interpolations
// This will overflow and go back to zero every 50 days if left on continuously, should be fine haha
uint32_t iterationCount = 0;

void setup()
{
  Serial.begin(115200); // opens serial port, sets data rate to 115200 bps
  FastLED.addLeds<WS2812, PIN>(currentLEDState, pixel_count);
  FastLED.show();
  pinMode(LED_BUILTIN, OUTPUT);
}

// Will take a few calls to remove all in certain cases, not really concerned about it though
void removeCompletedAnimations()
{
  StripAnimation animation;
  for (uint8_t i = 0; i < stripAnimationLinkedList.size(); i++)
  {
    animation = stripAnimationLinkedList.get(i);
    if (animation.isMarkedForRemoval)
    {
      stripAnimationLinkedList.remove(i);
    }
  }
}

void setPixelRangeToColor(uint8_t startPixel, uint8_t endPixel, struct CRGB color)
{
  for (uint8_t i = startPixel; i <= endPixel; i++)
  {
    currentLEDState[i] = color;
  }
}

void updateCurrentStateBasedOnCurrentAnimations()
{
  StripAnimation animation;
  for (uint8_t i = 0; i < stripAnimationLinkedList.size(); i++)
  {
    animation = stripAnimationLinkedList.get(i);
    if (animation.isMarkedForRemoval)
    {
      setPixelRangeToColor(
          animation.instruction.startPixel,
          animation.instruction.endPixel,
          colorPresets[COLOR_BLANK]);
    }
    else
    {
      uint32_t finishTime = animation.startTimeMillis + animation.animationDurationMillis;

      // This looks like a bug hot-spot if I've ever seen one, look out for this...
      float linearAnimationProgress = 1;
      if (finishTime > iterationCount)
      {
        uint8_t progressMillis = iterationCount - animation.startTimeMillis;
        linearAnimationProgress = (float)progressMillis / (float)animation.animationDurationMillis;
      }
      // I'm distrustful of floating point numbers as you can see. This probably isn't needed.
      if (linearAnimationProgress > 1)
      {
        linearAnimationProgress = 1;
      }
      linearAnimationProgress = easing_easeOutCubic(linearAnimationProgress);

      uint8_t pixelRangeSize = animation.instruction.endPixel - animation.instruction.startPixel;
      setPixelRangeToColor(
          animation.instruction.startPixel,
          animation.instruction.startPixel + uint8_t(pixelRangeSize * linearAnimationProgress),
          colorPresets[animation.instruction.colorPreset]);
    }
  }
  removeCompletedAnimations();
}

void markAnimationForRemovalByStartAndEndPixel(uint8_t startPixel, uint8_t endPixel)
{
  StripAnimation animation;
  for (uint8_t i = 0; i < stripAnimationLinkedList.size(); i++)
  {
    animation = stripAnimationLinkedList.get(i);
    if (animation.instruction.startPixel == startPixel && animation.instruction.endPixel == endPixel)
    {
      // Just update the local copy and set it, not bothering with pointers until I'm more comfortable with them
      animation.isMarkedForRemoval = true;
      stripAnimationLinkedList.set(i, animation);
    }
  }
}

void loop()
{
  serial_checkForNewStripInstruction();
  // serial_hasNewStripInstruction will be set to true if serial_checkForNewStripInstruction() gets a full instruction from the serial port
  if (serial_hasNewStripInstruction)
  {
    // Calling serial_getNewSerialInstruction() will set serial_hasNewStripInstruction back to false
    StripInstruction instruction = serial_getNewSerialInstruction();
    if (instruction.colorPreset != COLOR_BLANK)
    {
      StripAnimation animation = {
          instruction,
          iterationCount,
          defaultAnimationDurationMillis,
          false};
      stripAnimationLinkedList.add(animation);
      Serial.println("+A");
    }
    // TODO (maybe?)
    // else if (isReplacingExistingAnimation) { ... }
    else
    {
      markAnimationForRemovalByStartAndEndPixel(instruction.startPixel, instruction.endPixel);
    }
  }

  if (iterationCount % displayRate == 0)
  {
    updateCurrentStateBasedOnCurrentAnimations();
    FastLED.show();
  }

  iterationCount++;
  delay(loopDelay);
}