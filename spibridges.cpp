// Adafruit Motor shield library
// copyright Adafruit Industries LLC, 2009
// this code is public domain, enjoy!


#if (ARDUINO >= 100)

#include "Arduino.h"

#else
#if defined(__AVR__)
#include <avr/io.h>
#endif
#include "WProgram.h"
#endif

#include "spibridges.h"

template<unsigned clkpin, unsigned datapin, unsigned cspin>
class SoftSpi {
  OutputPin <clkpin> CK;//todo: cpol param for clock, and add an idle state for it as well.
  OutputPin <datapin> D;
  OutputPin <cspin> CS;

  void beIdle() {
    CS = 1;
    CK = 1;
  }

  //msb first
  void send(unsigned data, unsigned numbits = 8) {
    CS = 0;
    unsigned picker = 1 << (numbits - 1);
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wfor-loop-analysis"
    do {
      CK = 0;
      D = (data & picker) ? 1 : 0;
      picker >>= 1; //placed here to increase data setup time before clock edge, helps if driving an isolator.
      CK = 1;
    } while (picker);
#pragma clang diagnostic pop
    CS = 1;
  }
};

/** we take advantage of the compatible timing between a typical spi cs and the HC595 output register clock to pretend that the HC595 is a normal spi device */
template<unsigned clkpin, unsigned datapin, unsigned rckpin, unsigned cspin>
class HC595 : SoftSpi<clkpin, datapin, rckpin> {
  OutputPin <rckpin> OE;

  void start(bool free = false) {
    phasors.send(free ? ~0 : 0);
    OE = 0;
  }
};

static bool greylsb(byte step) {
  return ((step >> 1 ^ step) & 1) != 0;
}

static bool greymsb(byte step) {
  return (step & 2) != 0;
}


//From V1.2 schematic:
//msb..lsb
//43421123
//BBABBAAA

class SpiDualBridgeBoard_Impl {
  const HC595<4, 8, 12, 7> phasors;
  uint8_t phases;

public:
  DuplicateOutput<11, 3> enfirst;
  DuplicateOutput<6, 5> ensecond;

  void start(bool free = false) {
    phasors.start(free);
    enfirst = 1;
    ensecond = 1;
  }

  void setBridge(bool second, bool x, bool y) {
    if (second) {
      phases &= 0x78;//2nd bridge is inverse mask of first
      phases |= x ? (1 << 0) : (1 << 6); //not putting the ternary in the shift as cortexm parts can load with constant shift.
      phases |= y ? (1 << 5) : (1 << 7);
    } else {
      phases &= ~0x78;
      phases |= x ? (1 << 2) : (1 << 3);
      phases |= y ? (1 << 1) : (1 << 4);
    }
    phasors.send(phases);
  }

  void setBridge(bool second, uint8_t phase) {
    setBridge(second, greymsb(phase), greylsb(phase));
  }

  void power(bool second,bool on) {
    if (second) {
      ensecond = on; //leave the phases alone.
    } else {
      enfirst = on; //leave the phases alone.
    }
  }
};

SpiDualBridgeBoard_Impl theBoard;//there can be only one, it takes up too many pins for two.

namespace SpiDualBridgeBoard {
  void start(bool free = false) {
    theBoard.start(free);
  }

  void setBridge(bool x, bool y) const {
    theBoard.setBridge(second, x, y);
  }

  void setBridge(uint8_t phase) const {
    theBoard.setBridge(second, phase);
  }

/** a hard kill tries to lock the rotor, a soft one lets it spin */
  void power(bool on) {
    theBoard.power(second,on);
  }
};
/*
Brutal control pinout, might as well be random.

From reading code:
msb..lsb
34321124
BBABBAAA


From V1.2 schematic:  (3 and 4 swapped)
msb..lsb
43421123
BBABBAAA

D7 en pulled up.
D4 clock
D8 data
D12 latch

pwm1a&B goto servo connectors, D9,D10
pwm2A&B goto M1&2, D11,D3
pwm0A&B goto M3&4, D6,D5


//2,13 available (also 0,1 uart)


*/

//void AF_Stepper::setSpeed(uint16_t rpm) {
//  usperstep = 60000000 / ((uint32_t)revsteps * (uint32_t)rpm);
//  steppingcounter = 0;
//}
