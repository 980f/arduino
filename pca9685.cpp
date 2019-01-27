#include "pca9685.h"
#include <Arduino.h> //to get hardware options.

enum Register {
  //names from manual
  Mode1 = 0x0,
  Mode2 = 0x1,
  //Sub1= 0x2,
  //Sub2= 0x3,
  //Sub3= 0x4,
  //AllCall=0x5,
  Prescale = 0xFE,
  LedBase = 0x6,
  //AllOn= 0xFA  //same as led 0xF4, or 244.
};

enum ControlBits {
  Restarter = 1 << 7,
  //1<<6: ext clock not yet supported
  AutoIncrement = 1 << 5,
  Sleeper = 1 << 4,
  //others are generated algorithmically from an id #
};


PCA9685::PCA9685( uint8_t addr, unsigned which): WireWrapper(addr, which) {
  //#done
}

void PCA9685::begin(uint8_t mode2value) {
  WireWrapper::begin();
  send(Mode1, uint8_t(Restarter | AutoIncrement | Sleeper));
  send(Mode2, mode2value);
  send(Prescale, fromHz(1000)); //mimic Adafruit for ease of adoption.
  delayMicroseconds(wake());
}

uint8_t PCA9685::updatemode(uint8_t ones, uint8_t zeroes) {
  uint8_t mode = inp<uint8_t>(Mode1);
  send(Mode1, ((mode | ones) & ~zeroes));//#parens required to ensure order of operations. without them the ones&~zeroes combined first, then were or'd into mode, preventing us from clearing bits.
  return mode;
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
  prescaleval /= hz * 4096;

  if (prescaleval >= 256) {//register is 8 bits
    return 255;
  }
  
  if (prescaleval < 4) {
    return 3;//because hardware will convert it to this.
  }
  return uint8_t(prescaleval) - 1;//so says the manual.
}

void PCA9685::setPrescale(uint8_t bookvalue) {
  uint8_t oldmode =  updatemode((1 << 4), 0); // reset off, sleep on, see footnote [1] under register address table.
  send(Prescale, bookvalue); // set the prescaler
  send(Mode1, oldmode);
  delayMicroseconds(500);//in case oscillator gets whacked. Better to not whack it.
  //not this guy's bailiwick:   outp(Mode1, oldmode | (1<<7) | (1<<5));  //bit 7 enables restart logic,  bit 5 is auto increment.
}

//this declaration is convenient, to set which template verions of twi.write() to invoke.
static const uint16_t FULL = 4096;
/**
  when bit 12 is set in either value then the output is either full on or full off,
  if both requested full off wins. The other 12 bits don't matter.
*/
void PCA9685::setChannel(uint8_t which, uint16_t on, uint16_t off = 0) {
  if (~which == 0) { //cheat for ALL
    which = 244; //results in correct address
  } else if (which > 15) {
    return; //invalid address
  }

  uint8_t ledaddr = LedBase + 4 * which;
  //normalize value for good luck (robustness against future changes to the chip).
  if (off > 4095) { //full off. Tested first as the hardware itself also tests it first.
    send(ledaddr + 2, FULL);
    return;
  }
  if (on > 4095) { //full on
    send(ledaddr, FULL);
    return;
  }
  //else the values are in the operational range.
  send(ledaddr, on, off);
}

unsigned PCA9685::sleep() {
  uint8_t modewas = updatemode( (1 << 7) | (1 << 4), 0); //ack restart, set sleep
  return 0;//make it look like wake so that we can use ternary to invoke wake else sleep
}


unsigned PCA9685::wake() {
  uint8_t modewas = updatemode( (1 << 7), (1 << 4)); //ack restart, no sleep
  if (modewas & (1 << 4)) {
    return 500;//wakeup time from sleep
  }
  return 0;
}

void PCA9685::setAddress(uint8_t which, uint8_t sevenbit) {
  which &= 3; //guard against bad value.
  //set a register 2,3,4, or 5 where 5 is ALL call.
  send(5 - which, sevenbit << 1); //this mapping makes bit setting below easy.
  updatemode( (1 << which) , 0); // set related control bit
}
