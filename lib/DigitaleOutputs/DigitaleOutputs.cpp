#include "DigitaleOutputs.h"
#include <Wire.h>
#include <Adafruit_MCP23X17.h>

namespace {
  Adafruit_MCP23X17 mcpOut;     // dieses Modul besitzt den Ausgangs-Chip
  bool gOk = false;

  void schalte(uint8_t pin, bool an) {
    if (gOk) mcpOut.digitalWrite(pin, an ? HIGH : LOW);   // active-high
  }
}

void digitaleOutputsBegin() {
  gOk = mcpOut.begin_I2C(ADDR_MCP_OUT, &Wire);
  if (gOk)
    for (uint8_t p = 0; p < 16; p++) {
      mcpOut.pinMode(p, OUTPUT);
      mcpOut.digitalWrite(p, LOW);    // Transistoren aus beim Start
    }
}

bool digitaleOutputsOk() { return gOk; }

void heizen(Etage etage, bool an)  { schalte(MCPOUT_HEIZEN[(uint8_t)etage],  an); }
void kuehlen(Etage etage, bool an) { schalte(MCPOUT_KUEHLEN[(uint8_t)etage], an); }
