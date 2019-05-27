#include <Adafruit_NeoPixel.h>

#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
#define pixel_count 150

// Memory (SRAM) usage table:
// 450 bytes: Adafruit_NeoPixel strip
// 600 bytes: RGB currentState
// 600 bytes: RGB destinationState
// Total: 1,650 bytes
// Uno total: 2048 bytes
// Remaining: 398 bytes

#define SET_DESTINATION_STATE_SERIAL_HEADER_BYTE 0
#define SET_DESTINATION_STATE_SERIAL_BYTE_LENGTH 3

#define verboseSerialOutput true

// 24 byte Serial Strip Instruction
// These will be read out from serial port one at a time
// to prevent memory overflow
struct StripInstruction
{
  uint8_t startPixel;
  uint8_t endPixel;
  uint8_t colorPreset;
};

struct RGB
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

enum EColorPresets
{
  COLOR_BLANK = 0,
  COLOR_SOFT_RED = 1,
};

// Extraneous duplication here
RGB colorPresets[2] = {
    {0, 0, 0}, // 0 COLOR_BLANK
    {6, 2, 2}  // 1 COLOR_SOFT_RED
};

// 3 bytes SRAM per RGB value
RGB currentState[pixel_count];
RGB destinationState[pixel_count];

int loopDelay = 1;
int interpolationRate = 8; // Every n loops
uint16_t iterationCount = 0;

// 3 bytes SRAM per strip pixel
Adafruit_NeoPixel strip = Adafruit_NeoPixel(pixel_count, PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
}

void loop()
{
  // Test code
  StripInstruction instruction;
  if (iterationCount == 1000)
  {
    iterationCount = 0;
  }

  if (iterationCount == 0)
  {
    instruction.startPixel = 0;
    instruction.endPixel = 20;
    instruction.colorPreset = COLOR_BLANK;
    setDestinationStateFromStripInstruction(instruction);
  }

  if (iterationCount == 500)
  {
    instruction.startPixel = 0;
    instruction.endPixel = 20;
    instruction.colorPreset = COLOR_SOFT_RED;
    setDestinationStateFromStripInstruction(instruction);
  }
  // end test code
  readSerialDataIfAvailable() if (iterationCount % interpolationRate == 0)
  {
    interpolateCurrentStateTowardsDestinationState();
    setStripToCurrentState();
  }
  iterationCount++;
  delay(loopDelay);
}

// Little helper function that prints out incoming data if verboseSerialOutput is true
uint8_t readSerial()
{
  // Serial buffer is 64 bytes
  uint8_t incomingByte = Serial.read();
  if (verboseSerialOutput)
  {
    Serial.print("I received: ");
    Serial.println(incomingByte, DEC);
  }
  return incomingByte;
}

void readSerialDataIfAvailable()
{
  if (Serial.available() == 0)
    return;

  if (incomingByte == SET_DESTINATION_STATE_SERIAL_HEADER_BYTE)
  {
    StripInstruction instructions;
    instructions.startPixel = readSerial();
    instructions.endPixel = readSerial();
    instructions.colorPreset = readSerial();
    setDestinationStateFromStripInstruction(instructions)
  }
}

void setDestinationStateFromStripInstruction(StripInstruction instruction)
{
  for (uint8_t i = instruction.startPixel; i <= instruction.endPixel; i++)
  {
    destinationState[i] = colorPresets[instruction.colorPreset];
  }
}

// Not "true" interpolation, but much less memory intensive.
void interpolateCurrentStateTowardsDestinationState()
{
  RGB currentRGBValue;
  RGB destinationRGBValue;
  for (uint8_t i = 0; i < strip.numPixels(); i++)
  {
    currentRGBValue = currentState[i];
    destinationRGBValue = destinationState[i];

    if (currentRGBValue.r != destinationRGBValue.r)
    {
      if (currentRGBValue.r < destinationRGBValue.r)
      {
        currentRGBValue.r++;
      }
      else
      {
        currentRGBValue.r--;
      }
    }

    if (currentRGBValue.g != destinationRGBValue.g)
    {
      if (currentRGBValue.g < destinationRGBValue.g)
      {
        currentRGBValue.g++;
      }
      else
      {
        currentRGBValue.g--;
      }
    }

    if (currentRGBValue.b != destinationRGBValue.b)
    {
      if (currentRGBValue.b < destinationRGBValue.b)
      {
        currentRGBValue.b++;
      }
      else
      {
        currentRGBValue.b--;
      }
    }
    currentState[i] = currentRGBValue;
  }
}

void setStripToCurrentState()
{
  for (uint8_t i = 0; i < strip.numPixels(); i++)
  {
    strip.setPixelColor(i, currentState[i].r, currentState[i].g, currentState[i].b);
  }
  strip.show();
}