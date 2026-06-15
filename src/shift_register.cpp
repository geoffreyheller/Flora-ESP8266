#include "shift_register.h"

static uint8_t latchPin = 15;

static void IRAM_ATTR shiftSetDataBits(uint16_t bits) {
  const uint32_t mask = ~((SPIMMOSI << SPILMOSI) | (SPIMMISO << SPILMISO));
  bits--;
  SPI1U1 = ((SPI1U1 & mask) | ((bits << SPILMOSI) | (bits << SPILMISO)));
}

static void IRAM_ATTR shiftTransferByte(uint8_t data) {
  while (SPI1CMD & SPIBUSY) {}
  shiftSetDataBits(8);
  SPI1W0 = data;
  SPI1CMD |= SPIBUSY;
  while (SPI1CMD & SPIBUSY) {}
}

void shiftRegisterInit(uint8_t mosi, uint8_t sck, uint8_t latch) {
  latchPin = latch;

  pinMode(mosi, SPECIAL);
  pinMode(12, SPECIAL); // MISO — required for HSPI even when unused
  pinMode(sck, SPECIAL);
  pinMode(latchPin, OUTPUT);
  digitalWrite(latchPin, LOW);

  GPMUX &= ~(1 << 9);
  SPI1C = 0;
  SPI1CLK = 0x009c1001; // SPI_CLOCK_DIV16 = 1 MHz
  SPI1U = SPIUMOSI | SPIUDUPLEX | SPIUSSE;
  SPI1U1 = (7 << SPILMOSI) | (7 << SPILMISO);
  SPI1C1 = 0;

  // SPI_MODE0, MSBFIRST
  SPI1U &= ~SPIUSME;
  SPI1P &= ~(1 << 29);
  SPI1C &= ~(SPICWBO | SPICRBO);
}

void IRAM_ATTR shiftWriteBytes(const volatile uint8_t* data, size_t count) {
  for (size_t i = 0; i < count; i++) {
    shiftTransferByte(data[count - 1 - i]);
  }

  GPOS = 1 << latchPin;
  GPOC = 1 << latchPin;
}
