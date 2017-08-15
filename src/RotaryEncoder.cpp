#include "RotaryEncoder.h"

RotaryEncoder::RotaryEncoder(uint8_t pinA, uint8_t pinB, uint8_t controllerNumber, uint8_t channel, int speedMultiply, uint8_t pulsesPerStep, relativeCCmode mode) : enc(pinA, pinB) // Constructor
{
  this->controllerNumber = controllerNumber;
  this->channel = channel;
  this->speedMultiply = speedMultiply;
  this->pulsesPerStep = pulsesPerStep;
  this->mode = mode;
}

void RotaryEncoder::refresh() // Check if the encoder position has changed since last time, if so, send the relative movement over MIDI
{
  long value = enc.read();
  long difference = (value - oldVal) / pulsesPerStep;
  while (difference != 0)
  {
    if (difference > 15)
      difference = 15;
    if (difference < -15)
      difference = -15;

    uint8_t msgVal = mapRelativeCC(difference * speedMultiply, mode);
    USBMidiController.send(CC, channel, controllerNumber, msgVal);
    oldVal += difference * pulsesPerStep;

    value = enc.read();
    difference = (value - oldVal) / pulsesPerStep;
  }
}

uint8_t RotaryEncoder::twosComplement7bit(int8_t value) // Convert an 8-bit two's complement integer to a 7-bit two's complement integer
{
  return (value & 0b10111111) - (value & 0b01000000);
}

uint8_t RotaryEncoder::twosComplementTo7bitSignedMagnitude(int8_t value) // Convert an 8-bit two's complement integer to 7-bit sign-magnitude format
{
  uint8_t mask = value >> 7;
  uint8_t abs = (value + mask) ^ mask;
  uint8_t sign = mask & 0b01000000;
  return (abs & 0b00111111) | sign;
}

uint8_t RotaryEncoder::mapRelativeCC(int8_t value, relativeCCmode mode) // Convert an 8-bit two's complement integer to a 7-bit value to send over MIDI
{
  if (mode == TWOS_COMPLEMENT)
    return twosComplement7bit(value);
  if (mode == BINARY_OFFSET)
    return value + 64;
  if (mode == SIGN_MAGNITUDE)
    return twosComplementTo7bitSignedMagnitude(value);
  return 0;
}