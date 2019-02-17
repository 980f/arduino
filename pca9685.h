#pragma once  ////(C) 2019 Andy Heilveil, github/980F

#include "wirewrapper.h" //wrap usage of TwoWire class

/** PCA9685 interface, using Arduino Wire library for i2c access.
    The Strangest thing is the 'shadow' feature, where one is slaved to another. This can be used for a hot spare, split load without redoing software when redistributing the load, or with a little thought differential output or even just physically independent status light driver.
*/
class PCA9685 {
    /** which I2C bus and device*/
    WireWrapper ww;
  private: //internal registers, see device manual for names.
    WIred<uint8_t> Mode1;   //maydo: create this locally, not called urgently.
    WIred<uint8_t> Prescale;//maydo: create this locally, not called urgently.
    //addresses and channels are dynamically generated, they consume too much ram to force all users to allocate them. Construction on demand (on stack) is cheap.
    //redundant/parallel device. For late in the game load splitting.

  public:
    /** device i2C address, 7-bit, @param which is which i2c bus, 0 for the default, 1 for the 2nd available on some of the bigger processors. */
    PCA9685( uint8_t addr = 0x40, unsigned which = 0);
    PCA9685 *shadow = nullptr;
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

      default hz is Adafruit choice.
    */
    bool begin(uint8_t mode2value = 0, unsigned hz = 1000);

    /** @returns the number of microseconds you should wait to ensure the device is ready for new values. */
    unsigned wake();

    /** stop emissions. @returns 0 to make it have same signature as wake().  */
    unsigned sleep();

    /** compute the prescalar value for pwm base rate in Hz. Not static as someday we will support ext clock input at which time we will have a member for the freq.
        For default oscillator we could support centiHz without overflowing the internal calculation.
    */
    uint8_t fromHz(unsigned hz);

    /** set the prescalar, note: device is put to sleep inside this routine, if you don't set @param andRun then you will need to wake it up after you have done your other settings, we do this to allow a gitch-free startup.*/
    void setPrescale(uint8_t bookvalue, bool andRun = false);

    /** We only support nominal Hz here. If reacting to something, such as a control knob, work in prescale units directly. No point in dragging in the whole floating point library for the rare user.   */
    void setPWMFreq(unsigned hz, bool andRun = true) ;

    /** @returns effective prescalar, not the literal hardware value.*/
    unsigned getPrescale() {
      uint8_t raw = Prescale; //#compiler gets confused if we try to inline this.
      return 1 + raw;
    }

    /** set one of 3 alternative I2C addresses, or if which =0 configure 'all call' address. This sets the address and the related enable bit.
      If the syntax were easier we would add this to begin. */
    void setAddress(uint8_t which, uint8_t sevenbit);

    /** for debug use: set given range of channels to their channel number. */
    void idChannels(uint8_t lowest = 0, uint8_t highest = 15);

  public: //per channel stuff
    /** modify a waveform. pass which=~0 for all. */
    void setChannel(uint8_t which, uint16_t on, uint16_t off = 0);

    /** modify just the off */
    void setWidth(uint8_t which, uint16_t endvalue);

    /** modify just the off */
    void setPhase(uint8_t which, uint16_t endvalue);


    class Output {
        PCA9685 &dev;
      public:
        const uint8_t which;
        Output (PCA9685 &dev, uint8_t which): dev(dev), which(which) {}
        void operator =(uint16_t endvalue) {
          dev.setWidth(which, endvalue);
        }
        void setPhase(uint16_t onvalue) {
          dev.setPhase(which, onvalue);
        }
    };

  private:
    /** alter the mode1 register, setting bits where there are 1's in @param ones and clearing bits where @param zeroes has <em>ones</em> */
    uint8_t updatemode(uint8_t ones, uint8_t zeroes) ;

};
