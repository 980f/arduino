#include "Arduino.h"  //IDE needs this
#include "spibridges.h"
#include "pinclass.h"
#include "bitbanger.h"

#ifdef SoftSpiSpew
#include "chainprinter.h"
static ChainPrinter sdbg(SoftSpiSpew, true);
#else
#define sdbg(...)
#endif

template<unsigned clkpin, unsigned datapin, unsigned cspin>
class SoftSpi {
    OutputPin<clkpin> CK;//todo: cpol param for clock, and add an idle state for it as well.
    OutputPin<datapin> D;
    OutputPin<cspin, LOW> CS;
  public:
    void beIdle() const {
      CS = 0;
      CK = 1;
    }

    //msb first
    void send(unsigned data, unsigned numbits = 8) const {
      sdbg("Send:", BITLY(data));
      CS = 1;
      unsigned picker = 1 << (numbits - 1);
      do {
        CK = 0;
        D = (data & picker) ? 1 : 0;
        picker >>= 1; //placed here to increase data setup time before clock edge, helps if driving an isolator.
        CK = 1;
      } while (picker);
      CS = 0;
    }

};

/** we take advantage of the compatible timing between a typical spi cs and the HC595 output register clock to pretend that the HC595 is a normal spi device */
template<unsigned clkpin, unsigned datapin, unsigned rckpin, unsigned oepin>
struct HC595 : SoftSpi<clkpin, datapin, rckpin> {
  OutputPin<oepin, LOW> OE;
  using Super = SoftSpi<clkpin, datapin, rckpin>;

  void start() const {
    Super::beIdle();
    OE = 1;
  }

  void send(unsigned data, unsigned numbits = 8) const {
    Super::send(data, numbits);
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
    enum : uint8_t {
      nibbler = BitWad<7, 6, 5, 0>::mask, //bits for 'second' motor
    };
  public:
    //placeholders for the enable bits:  //we turn them both on or off as a total power on/off for the motor.
    //half step has to be coordinated with the stepping so there is no value in using these bits for that purpose.
    //their main value is to reduce heat when the motor is not moving/ moving very slowly.
    DuplicateOutput<11, 3> enfirst;
    DuplicateOutput<6, 5> ensecond;

    void start(bool free = false) {
      phases = free ? ~0U : 0; //matters to unipolar rig.
      phasors.send(phases);
      phasors.start();
      enfirst = 1;
      ensecond = 1;
    }

    void setBridge(bool second, bool x, bool y) {
      if (second) {
        phases &= ~nibbler;//2nd bridge is inverse mask of first
        phases |= x ? bit(0) : bit(6); //not putting the ternary in the bit() as cortexm parts can load with constant shift.
        phases |= y ? bit(5) : bit(7); //.. and doing it this way leans on the compiler to do the bit() at compile time.
      } else {
        phases &= ~~nibbler;
        phases |= x ? bit(2) : bit(3);
        phases |= y ? bit(1) : bit(4);
      }
      phasors.send(phases);
    }

    void setBridge(bool second, uint8_t phase) {
      setBridge(second, greymsb(phase), greylsb(phase));
    }

    void power(bool second, bool on) {
      if (second) {
        ensecond = on; //leave the phases alone.
      } else {
        enfirst = on; //leave the phases alone.
      }
    }

    bool isPowered(bool second) const {
      return bitFrom(phases, second ? ensecond : enfirst);
    }

    //return last phase sent, if fed back in to setBridge(,...) will be no change
    uint8_t phase(bool second) {
      //assumes not in drift state!
      bool graymsb = bitFrom(phases, second ? 0 : 2);
      bool graylsb = bitFrom(phases, second ? 5 : 1);
      return graymsb << 1 | (graylsb ^ graymsb);
    }

};

SpiDualBridgeBoard_Impl theBoard;//there can be only one, it takes up too many pins for two.

void SpiDualBridgeBoard::start(bool free) {
  theBoard.start(free);
}

void SpiDualBridgeBoard::setBridge(bool second, bool x, bool y) {
  theBoard.setBridge(second, x, y);
}

void SpiDualBridgeBoard::setBridge(bool second, uint8_t phase) {
  theBoard.setBridge(second, phase);
}

/** a hard kill tries to lock the rotor, a soft one lets it spin */
void SpiDualBridgeBoard::power(bool second, bool on) {
  theBoard.power(second, on);
}


bool SpiDualPowerBit::operator =(bool on)const {
  theBoard.power(second, on);

}
SpiDualPowerBit::operator bool()const {
  return theBoard.isPowered(second);
}


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

  servo connectors: D9,D10 oc1A|-oc4B, oc1B|oc4B
  M1&2 enables: D11,D3 oc0a|oc1C, oc0B
  M3&4 enables: D6,D5  oc4D,oc4A|oc3A


  //2,13 available (also 0,1 uart)


*/

//void AF_Stepper::setSpeed(uint16_t rpm) {
//  usperstep = 60000000 / ((uint32_t)revsteps * (uint32_t)rpm);
//  steppingcounter = 0;
//}
