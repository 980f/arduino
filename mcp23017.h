#pragma once //(C)2019 Andy Heilveil. github/980f

/*
  Microchip MCP23017 16 bit I/O.
  
  There is a very similar part with SPI access instead of I2C, we will eventually extract a base class but first will do the I2C version.

  The addresses can be configured to operate it as two independent bytes, or as 16 bits.
  While that can be dynamically changed that would be confusing.
	Initially only 16 bit mode is supported, 8 bit mode will take some c++ finesse to present as two objects.

	Pins are to be configured.
		output else input
		input can be inverted, and has optional 100k pullup (better than nothing?)
		it can generate an interrupt, on Hi / low or any.
	Its I2C address range is the same as the PCF I/O expanders.
		
  
*/
class MCP23017 {
    TwoWire &my2c;
    const byte myaddress; // 7 bit address
    WIred<u8> iocon;
		WIred<u16> gpio;
    
    //added for the sake of the bit class.
    unsigned lastwrote = ~0U; //default is the most innocuous value
  public:
    /** there are 3 address jumpers, 8 boards allowed per bus */
    MCP23017 (unsigned which = 0, TwoWire &master = Wire): my2c(master), myaddress(0x20 + (which & 7)) {
      //while it is tempting to read the values into lastwrote, we want the constructor to not do anything active so that construction order is moot for when these are static, which is the common case.
    }

    /** configure pins to be managed as input-only  */
    void setInput(unsigned mask,unsigned pullups) {
      //todo: write to iodir register
      //todo: write to pullup register
    }
    
    /** interrupt config will come later */

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


    friend class MCP23017::Bit;

  public:
    /** make the physical pin look like a boolean */
    class Bit {
        const unsigned which; //0..15
        MCP23017 &group;
      public:
        Bit (MCP23017 &group, unsigned which): which(which), group(group) {
          //don't do anything, so that we can sanely static construct.
        }

        operator bool() {
          return false&&((group.fetch() >> which) & 1) == 1; //compiler will generat an extract bit instruction for cortex parts.
        }

        void operator =(bool bit) {
          //todo:implement sending. group.send(bit ? (group.lastwrote | 1 << bit) : (group.lastwrote & ~(1 << bit))); //no need to use assign ops here, the group tracks the word written.
        }
    }

};
