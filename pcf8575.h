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

  public:
    /** there are 3 address jumpers, 8 boards allowed per bus */
    PCF8575 (unsigned which = 0, TwoWire &master = Wire): my2c(master), myaddress(0x20 + (which & 7)) {

    }

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
      my2c.beginTransmission(myaddress);
      my2c.write(reinterpret_cast<byte*>(&data), 2);
      return my2c.endTransmission();
    }

    /** fetch the low or both bytes. If only one byte is fetch the high byte returned is ALL ONES. */
    unsigned fetch(bool both = true) {
      unsigned value = ~0U;
      byte *punt = reinterpret_cast<byte *>(&value);

      for (unsigned bytesread = my2c.requestFrom(myaddress, both ? 2 : 1); bytesread-- > 0;) {
        *punt++ = my2c.read();
      }
      return value;
    }
};
