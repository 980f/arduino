//(C) 2019 Andy Heilveil, github/980F
#include "pca9685.h"
#include <Arduino.h> //to get hardware options.

enum ControlBits : uint8_t {
  Restarter = 1 << 7,  //on read indicates restart advisable, write 1 to clear the flag
  //1<<6: ext clock not yet supported
  AutoIncrement = 1 << 5,  //OK to have this always set
  Sleeper = 1 << 4,
  //others are generated algorithmically from an id #
};


PCA9685::PCA9685( uint8_t addr, unsigned which):
  ww(addr, which),
  Mode1(ww, 0x00),
  Prescale(ww, 0xFE)
{
  //#done
}

void PCA9685::begin(uint8_t mode2value, unsigned hz) {
  ww.begin();
  Mode1 = (Restarter | AutoIncrement);//init the cached value and trust it henceforth
  ww.Write(1, &mode2value, 1); //this is the only reference to mode2 byte, so no fancy encoding.
  setPWMFreq(hz, true);
}

uint8_t PCA9685::updatemode(uint8_t ones, uint8_t zeroes) {
  return Mode1.modify(ones, zeroes);
}


unsigned PCA9685::sleep() {
  uint8_t modewas = updatemode( Restarter | Sleeper, 0); //ack restart, set sleep
  return 0;//make it look like wake so that we can use ternary to invoke wake else sleep
}


unsigned PCA9685::wake() {
  uint8_t modewas = updatemode( 0, Sleeper); //ack restart, no sleep
  if (modewas & Sleeper) {
    return 500;//wakeup time from sleep
  }
  return 0;
}

/* avoiding floating point (drags in a bunch of code):
  25000000 is 0x17d7840, that is 6 lsbs of zero.
  4096 has 12 lsbs of zero. so predivide each of those by 64:
  25E6 ->390625 or 0x 5F5E1, if we had 24 bit math we could use that
  4k -> 64 , 0x40

  min prescale is '3', hardware enforced.~1526Hz.
  max is 255, ~24Hz.
  ps+1 * 4k/25E6 = 1/hz, hz=25E6/(4k*(ps+1))
*/
uint8_t PCA9685::fromHz(unsigned hz) {
  uint32_t prescaleval = 25000000;//todo:1 optional support of external clock.
  prescaleval /= hz * 4096;//order of operations matters to precision

  if (prescaleval >= 256) {//register is 8 bits
    return 255;
  }

  if (prescaleval < 4) {
    return 3;//because hardware will convert it to this.
  }
  return uint8_t(prescaleval) - 1;//so says the manual.
}

void PCA9685::setPrescale(uint8_t bookvalue, bool andRun) {
  sleep();//required, prescale changes are ignored if not sleeping.
  Prescale = bookvalue;
  if (andRun) {
    delayMicroseconds(wake());
  }
}

void PCA9685::setPWMFreq(unsigned hz, bool andRun = true) {
  setPrescale(fromHz(hz), andRun);
}

//this declaration is convenient, to set which template verions of twi.write() to invoke.
static const uint16_t FULL = 4096;
/**
  when bit 12 is set in either value then the output is either full on or full off,
  if both requested full off wins. The other 12 bits don't matter.
  @param which is either 0 to 15 OR all ones for ALL.
*/
void PCA9685::setChannel(uint8_t which, uint16_t on, uint16_t off = 0) {
  if (~which == 0) { //cheat for ALL
    which = 244; //results in correct address, 244=0xFA-6 where 6 is the address for LED 0 and 0xFA is 'all'
  } else if (which > 15) {
    return; //invalid address
  }

  uint8_t ledaddr = 6 + 4 * which;//ON register address

  //normalize value for good luck (robustness against future changes to the chip).
  if (off >= FULL) { //full off. Tested first as the hardware itself also tests it first.
    WIred<uint16_t>(ww, ledaddr) = FULL;
    return;
  }
  if (on >= FULL) { //full on
    WIred<uint16_t>(ww, ledaddr + 2) = FULL;;
    return;
  }
  //else the values are in the operational range.
  WIred<uint32_t>(ww, ledaddr) = on; //a horrible hack to get a clean update of the pair.
}

void PCA9685::setWidth(uint8_t which, uint16_t endvalue) {
  if (~which == 0) { //cheat for ALL
    which = 244; //results in correct address, 244=0xFA-6 where 6 is the address for LED 0 and 0xFA is 'all'
  } else if (which > 15) {
    return; //invalid address
  }

  uint8_t ledaddr = 6  + 2 + 4 * which; //OFF register address
  WIred<uint16_t>(ww, ledaddr) = endvalue >= FULL ? FULL : endvalue;
}


//set a register 2,3,4, or 5 where 5 is ALL call.
void PCA9685::setAddress(uint8_t which, uint8_t sevenbit) {
  which &= 3; //guard against bad value.
  // 5-which mapping of which to register number makes bit setting below easy.
  WIred<uint16_t>(ww, 5 - which) = sevenbit << 1;
  updatemode( (1 << which) , 0); // set related control bit
}


/** set given range of channels to their channel number. */
void PCA9685::idChannels(uint8_t lowest = 0, uint8_t highest = 15) {
  for (unsigned chi = lowest; chi <= highest; ++chi) {
    setWidth(chi, chi << 8);
  }
}
//end of file.
