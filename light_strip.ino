#include <Adafruit_NeoPixel.h>

#define PIN 6

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
#define pixel_count 60

// Memory (SRAM) usage table:
// 450 bytes: Adafruit_NeoPixel strip
// 600 bytes: RGB currentState
// 600 bytes: RGB destinationState
// Total: 1,650 bytes
// Uno total: 2048 bytes
// Remaining: 398 bytes

#define SET_DESTINATION_STATE_SERIAL_BYTE_LENGTH 3

#define verboseSerialOutput false

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
uint8_t colorPresetsLength = 6;
RGB colorPresets[6] = {
    {0, 0, 0},   // 0 COLOR_BLANK
    {24, 8, 8},  // 1 COLOR_SOFT_RED
    {8, 24, 8},  // 2 COLOR_SOFT_GREEN
    {8, 8, 24},  // 3 COLOR_SOFT_BLUE
    {24, 8, 24}, // 4 COLOR_SOFT_?
    {24, 24, 8}, // 5 COLOR_SOFT_?
};

// 3 bytes SRAM per RGB value
RGB currentState[pixel_count];
RGB destinationState[pixel_count];

int loopDelay = 1;
int interpolationRate = 2; // Every n loops
uint16_t iterationCount = 0;

// 3 bytes SRAM per strip pixel
Adafruit_NeoPixel strip = Adafruit_NeoPixel(pixel_count, PIN, NEO_GRB + NEO_KHZ800);

void setup()
{
  Serial.begin(9600); // opens serial port, sets data rate to 9600 bps
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  if (iterationCount == 1000)
  {
    iterationCount = 0;
  }

  if (iterationCount == 0)
  {
    digitalWrite(LED_BUILTIN, HIGH);
  }

  else if (iterationCount == 500)
  {
    digitalWrite(LED_BUILTIN, LOW);
  }

  readSerialDataIfAvailable();
  if (iterationCount % interpolationRate == 0)
  {
    interpolateCurrentStateTowardsDestinationState();
    setStripToCurrentState();
  }
  iterationCount++;
  delay(loopDelay);
}

void onUnexpectedData()
{
  Serial.println("UNEX");
}

// Little helper function that prints out incoming data if verboseSerialOutput is true
uint8_t readSerial()
{
  // Serial buffer is 64 bytes
  uint8_t incomingByte = Serial.read();
  // if (verboseSerialOutput)
  // {
  //   Serial.println(incomingByte, DEC);
  // }
  return incomingByte;
}

// Anywhere from "0-0-0\n" to "255-255-255\n"
String inputString = "";
uint8_t inputFields[3];
uint8_t inputFieldsIndex = 0;

void readSerialDataIfAvailable()
{
  while (Serial.available() > 0)
  {
    int inChar = Serial.read();
    if (isDigit(inChar))
    {
      // convert the incoming byte to a char and add it to the string:
      inputString += (char)inChar;
    }
    else if ((char)inChar == '-')
    {
      inputFields[inputFieldsIndex] = inputString.toInt();
      inputFieldsIndex++;
      // Serial.println("X");
      // Serial.print(inputString);
      // Serial.println("X");
      inputString = "";
    }
    else if (inChar == '\n')
    {
      inputFields[inputFieldsIndex] = inputString.toInt();

      StripInstruction instruction;
      instruction.startPixel = inputFields[0];
      instruction.endPixel = inputFields[1];
      instruction.colorPreset = inputFields[2];
      setDestinationStateFromStripInstruction(instruction);

      inputFieldsIndex = 0;
      inputString = "";

      // Serial.println(inputString.toInt());
      // Serial.println("START");
      // Serial.print(instruction.startPixel);
      // Serial.print("-");
      // Serial.print(instruction.endPixel);
      // Serial.print("-");
      // Serial.print(instruction.colorPreset);
      // Serial.print(".");
      // clear the string for new input:
    }
  }
}

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
        if (iterationCount % 8 == 0)
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
        if (iterationCount % 8 == 0)
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
        if (iterationCount % 8 == 0)
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