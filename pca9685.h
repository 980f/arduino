
#pragma once

#include "wirewrapper.h" //wrap usage of TwoWire class

/** PCA9685 interface, using Arduino Wire library for i2c access.  */
class PCA9685: public WireWrapper {
  public:
    /** device i2C address, 7-bit, @param which is which i2c bus, 0 for the default, 1 for the 2nd available on some of the bigger processors. */
    PCA9685( uint8_t addr = 0x40, unsigned which = 0);

    /** set some defaults, starts operating at a nominal refresh rate to mimic adafruit library.
      argument is for output configuration byte for mode2 register, RTFM for now. Default value is that of powerup.
      1<<4 polarity when on by OE, typically only set when adding an NFet or Darlington between the device and the load.
      1<<3 high is output each channel as its set of 4 is completed, else update all when STOP happens. The latter makes the most sense.
      1<<2 high for totem pole drive (e.g. servo)
      1<<1 high for tri-state when off by OE
      1<<0 LOW to drive output low when off by OE
      some choices:
      servo direct drive:  4  (0b100)
      Typical LED low side drive, open when idle:
    */
    void begin(uint8_t mode2value = 0);

    /** @returns the number of microseconds you should wait to ensure the device is ready for new values. */
    unsigned wake();

    /** stop emissions. @returns 0 to make it have same signature as wake().  */
    unsigned sleep();

    /** compute the prescalar value for pwm base rate in Hz. Not static as someday we will support ext clock input at which time we will have a member for the freq.
        For default oscillator we could support centiHz without overflowing the internal calculation.
    */
    uint8_t fromHz(unsigned hz);

    /** set the prescalar, note: device is put to sleep inside this routine, you will need to wake it up after you have done your other settings, done to a gitch-free startup.*/
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

    /** set given range of channels to their channel number. */
    void idChannels(uint8_t lowest=0, uint8_t highest=15) {
      for (unsigned chi = highest; chi >= lowest; --chi) {
        setChannel(chi, 0, chi << 8);
      }
    }

  private:
    /** alter the mode1 register, setting bits where there are 1's in @param ones and clearing bits where @param zeroes has <em>ones</em> */
    uint8_t updatemode(uint8_t ones, uint8_t zeroes) ;

};
