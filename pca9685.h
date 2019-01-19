
#pragma once

#include "wirewrapper.h" //wrap usage of TwoWire class

/** PCA9685 interface */
class PCA9685: public WireWrapper {
  public:
    PCA9685( uint8_t addr = 0x40, unsigned which = 0);
    /** set some defaults, starts operating at a nominal refresh rate to mimic adafruit library.
      argument is for output configuration byte for mode2 register, RTFM for now. Default value is that of powerup. */
    void begin(uint8_t mode2value = 0);

    /** @returns the number of microseconds you should wait to ensure the device is ready for new values. */
    unsigned wake();
    /** stop emissions. @returns 0 to make it have same signature as wake().  */
    unsigned sleep();

    /** compute the prescalar value for pwm base rate in Hz. Not static as someday we will support ext clock input at which time we will have a member for the freq.
     *  we could support centiHz without overflowing the internal calculation.
    */
    uint8_t fromHz(unsigned hz);

    /** set the prescalar, note: device is put to sleep inside this routine, you will need to wake it up after you have done your other settings if you wish a gitch-free startup.*/
    void setPrescale(uint8_t bookvalue);

    /** We only support nominal Hz here. If reacting to something, such as a control knob, work in prescale units directly. No point in dragging in the whole floating point library for the rare user.   */
    void setPWMFreq(unsigned hz) {
      setPrescale(fromHz(hz));
    }

    /** modify a waveform. pass which=~0 for all. */
    void setChannel(uint8_t which, uint16_t on, uint16_t off = 0);

    /** set one of 3 alternative I2C addresses, or if which =0 configure 'all call' address. This sets the address and the related enable bit.
      If the syntax were easier we would add this to begin. */
    void setAddress(uint8_t which, uint8_t sevenbit);

  private:
    uint8_t updatemode(uint8_t ones, uint8_t zeroes) ;

};
