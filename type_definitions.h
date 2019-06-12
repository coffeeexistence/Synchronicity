#pragma once

struct StripInstruction
{
    uint8_t startPixel;
    uint8_t endPixel;
    uint8_t colorPreset;
};

struct StripAnimation
{
    StripInstruction instruction;
    uint32_t startTimeMillis;
    uint8_t animationDurationMillis;
    boolean isMarkedForRemoval;
};