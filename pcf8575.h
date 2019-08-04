#pragma once //(C)2019 Andy Heilveil. github/980f

/*
  PCF8575 16 bit I/O.
  weak high drive, write a 1 to allow input, but don't keep on writing it, each write operation pulses the line high for an I2C clock.
  The above suggests that if you have active input you should put them on the high byte, so that you can write just one byte to the low for controls.
*/
class PCF8575 {
    TwoWire &my2c;
    const byte myaddress; // 7 bit address
    unsigned inputs = 0xff00;//default: low byte output, high byte input
    //added for the sake of the bit class.
    unsigned lastwrote = ~0U; //default is the most innocuous value
  public:
    /** there are 3 address jumpers, 8 boards allowed per bus */
    PCF8575 (unsigned which = 0, TwoWire &master = Wire): my2c(master), myaddress(0x20 + (which & 7)) {
      //while it is tempting to read the values into lastwrote, we want the constructor to not do anything active so that construction order is moot for when these are static, which is the common case.
    }

    /** configure pins to be managed as input-only by this software module, the board itself doesn't have the concept. */
    void setInput(unsigned mask) {
      inputs = mask;
    }

    /** @returns whether device seems to be present */
    bool detect() {
      my2c.beginTransmission(myaddress);
      auto error = my2c.endTransmission();
      return !error;
    }

    /** sends 2 bytes of data, @param data given is OR'd herein with the input mask. */
    byte send(unsigned data) {
      data |= inputs;
      lastwrote = data; //record even if we fail to send.
      my2c.beginTransmission(myaddress);
      my2c.write(reinterpret_cast<byte*>(&data), 2);
      return my2c.endTransmission();
    }

    /** fetch the low or both bytes. If only one byte is fetched the high byte returned is ALL ONES. */
    unsigned fetch(bool both = true) {
      unsigned value = ~0U;
      byte *punt = reinterpret_cast<byte *>(&value);

      for (unsigned bytesread = my2c.requestFrom(myaddress, both ? 2 : 1); bytesread-- > 0;) {
        *punt++ = my2c.read();
      }
      return value;
    }


    friend class PCF8575::Bit;

  public:
    /** make the physical pin look like a boolean */
    class Bit {
        const unsigned which; //0..15
        PCF8575 &group;
      public:
        Bit (PCF8575 &group, unsigned which): which(which), group(group) {
          //don't do anything, so that we can sanely static construct.
        }

        operator bool() {
          return ((group.fetch() >> which) & 1) == 1; //compiler will generat an extract bit instruction for cortex parts.
        }

        void operator =(bool bit) {
          group.send(bit ? (group.lastwrote | 1 << bit) : (group.lastwrote & ~(1 << bit))); //no need to use assign ops here, the group tracks the word written.
        }
    }

};
