// Copyright 2020 Serdar Üşenmez
// Distributed under the terms of the GNU General Public License v3.0

#include <YetAnotherPcInt.h>
#include <Wire.h>

#define I2C_ADDRESS 0x04
#define CHANNEL_COUNT 10
#define ID_LENGTH 32

char g_sIdentification[ID_LENGTH] = "RcSamplerInterrupt 0.2";

uint8_t g_iChannelIndex[CHANNEL_COUNT];
const uint8_t g_iChannelPin[CHANNEL_COUNT] = { 2, 3, 4, 5, 6, 7, 8, 9, 10, 11 };

unsigned long g_iChannelPulseStart[CHANNEL_COUNT];
volatile uint16_t g_iChannelPulseWidth[CHANNEL_COUNT];

volatile uint8_t g_iReadCommand = 0;

void setup()
{
  Wire.onReceive(onReceive);
  Wire.onRequest(onRequest);
  Wire.begin(I2C_ADDRESS);

  for (int c = 0; c < CHANNEL_COUNT; ++c)
  {
    g_iChannelIndex[c] = c;
    
    uint8_t pin = g_iChannelPin[c];
    
    pinMode(pin, INPUT);

    g_iChannelPulseStart[c] = 0;
    g_iChannelPulseWidth[c] = 0;
  }

  for (int c = 0; c < CHANNEL_COUNT; ++c)
  {
    PcInt::attachInterrupt(g_iChannelPin[c], onChannelChanged, &g_iChannelIndex[c], CHANGE);
  }
}

void loop()
{
  // NOP
}

void onChannelChanged(uint8_t* ptrIdx, bool pinHigh)
{
  if (pinHigh)
  {
    g_iChannelPulseStart[*ptrIdx] = micros();
  }
  else
  {
    g_iChannelPulseWidth[*ptrIdx] = micros() - g_iChannelPulseStart[*ptrIdx];
  }
}

void onReceive(int numBytes)
{
  g_iReadCommand = Wire.read();
}

void onRequest()
{
  if (1 == g_iReadCommand)
  {
    // Send identification
    Wire.write((byte*)g_sIdentification, ID_LENGTH);
  }
  else
  {
    // Send sampled data
    Wire.write((byte*)g_iChannelPulseWidth, CHANNEL_COUNT * sizeof(uint16_t));
  }
}
