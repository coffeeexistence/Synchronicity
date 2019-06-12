#include "type_definitions.ino";

// Usage:
/*
    serial_checkForNewStripInstruction();
    // serial_hasNewStripInstruction will be set to true if serial_checkForNewStripInstruction() gets a full instruction from the serial port
    if (serial_hasNewStripInstruction)
    {
        // Calling serial_getNewSerialInstruction() will set serial_hasNewStripInstruction back to false
        StripInstruction instruction = serial_getNewSerialInstruction();
    }
*/

// Anywhere from "0-0-0\n" to "255-255-255\n"
String inputString = "";
uint8_t inputFields[3];
uint8_t inputFieldsIndex = 0;

bool serial_hasNewStripInstruction = false;
StripInstruction serial_newStripInstruction;

StripInstruction serial_getNewSerialInstruction()
{
    serial_hasNewStripInstruction = false;
    return serial_newStripInstruction;
}

void serial_checkForNewStripInstruction()
{
    if (Serial.available() > 0)
    {
        Serial.println("√");
    }
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
            inputString = "";
        }
        else if (inChar == '\n')
        {
            inputFields[inputFieldsIndex] = inputString.toInt();

            StripInstruction instruction;
            serial_newStripInstruction.startPixel = inputFields[0];
            serial_newStripInstruction.endPixel = inputFields[1];
            serial_newStripInstruction.colorPreset = inputFields[2];
            serial_hasNewStripInstruction = true;
            inputFieldsIndex = 0;
            inputString = "";
            return;
        }
    }
}