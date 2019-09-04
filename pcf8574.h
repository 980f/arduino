#pragma once //(C)2019 Andy Heilveil. github/980f
#include "wirewrapper.h" //I2C helper
/*
  PCF8574 8 bit I/O.
  weak high drive, write a 1 to allow input, but don't keep on writing it, each write operation pulses the line high for an I2C clock.
*/
class PCF8574: public WIredThing<uint8_t> {
   
    byte inputs = ~0;
    //added for the sake of the bit class.
    byte lastwrote = ~0; //default is the most innocuous value
  public:
    /** there are 3 address jumpers, 8 boards allowed per bus,  */
    PCF8574 (unsigned which = 0, unsigned whichMaster = 0): WIredThing<uint8_t>((0x20 + (which & 7)), whichMaster) {
      //while it is tempting to read the values into lastwrote, we want the constructor to not do anything active so that construction order is moot for when these are static, which is the common case.
    }

    /** configure pins to be managed as input-only by this software module, the board itself doesn't have the concept. */
    void setInput(unsigned mask) {
      inputs = mask;
    }

    /** sends  @param data  OR'd herein with the input mask. */
    uint8_t operator =(uint8_t data) {
      return WIredThing<uint8_t>::operator =(lastwrote = data | inputs); //record even if we fail to send.
    }

    /** set @param ones, clear @param zeroes, @returns value before these changes */
    uint8_t modify(uint8_t ones, uint8_t zeroes) {
      operator =((lastwrote | ones) & ~zeroes);
      return lastwrote;
    }

    /** set @param ones, clear @param zeroes, @returns value before these changes */
    uint8_t merge(uint8_t value, uint8_t mask) {
      return operator = (( (lastwrote & ~mask) | (value & mask)));
    }

    

    public:
    /** make the physical pin look like a boolean */
    class Bit {
      const unsigned mask; //0..15
      const bool polarity;
      PCF8574 &group;
public:
      Bit (PCF8574 & group, unsigned which, bool polarity = 1): mask(1 << which), polarity(polarity), group(group) {
        //don't do anything, so that we can sanely static construct.
      }

      operator bool() {
        //default to opposite of declared polarity for error return.
        return ((group.fetch(polarity ? 0 : ~0) & mask) != 0) == polarity; //compiler will generate an extract bit instruction for cortex parts.
      }

      void operator =(bool bit) {
        group.send(bit == polarity ? (group.lastwrote | mask) : (group.lastwrote & ~mask)); //no need to use assign ops here, the group tracks the word written.
      }
    };
friend class PCF8574::Bit;
};
