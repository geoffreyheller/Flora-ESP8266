#pragma once

#include <Arduino.h>

void shiftRegisterInit(uint8_t mosiPin, uint8_t sckPin, uint8_t latchPin);
void IRAM_ATTR shiftWriteBytes(const volatile uint8_t* data, size_t count);
