#pragma once

#include "Wire.h" //TwoWire class

/** condense use of I2C.
*/

class WireWrapper {
    friend class Transmission;
    uint8_t base;
    TwoWire &bus;
  public:

    WireWrapper( uint8_t addr, unsigned which = 0): base(addr),
#if defined(ARDUINO_SAM_DUE)
      bus(which ? Wire1, Wire) //so far only two are supported.
#else
      bus(Wire) //only one supported at present.
#endif
    {
      //#done
    }

    void begin() {
      bus.begin();
    }

    //  private: //internals of send() operation.
    //    template <typename T > void outp(T t) {
    //      //output low to high, will eventually want a flag for bigendian targets
    //      uint8_t *peeker = reinterpret_cast<uint8_t*>(&t);
    //      for (unsigned bc = sizeof(T); bc-- > 0;) {
    //        bus.write(*peeker++);
    //      }
    //    }
    //
    //    //peel off 1st arg and process it.
    //    template <typename First, typename ... Args> void more(const First &first, const Args & ... args) {
    //      outp(first);
    //      more(args...);
    //    }
    //
    //    //terminates vararg template iteration.
    //    template <typename ... > void more() {
    //      //#do nothing,
    //    }
    //
    //  public:
    //    /** send a bunch of values to the device address. Typically the first is a selector. */
    //    template <typename ... Args> void send(Args ... args) {
    //      bus.beginTransmission(base);
    //      more(args...);
    //      bus.endTransmission();
    //    }
    //
    //
    //    /** read a device register. @param addr is the selector in the device, not the i2c bus address which is already stored on this.
    //      until we manage to do the painful coding for varargs use packed structs to get a multiple return. */
    //    template <typename T > T inp(uint8_t addr) {
    //      bus.beginTransmission(base);
    //      bus.write(addr);
    //      bus.endTransmission(false);//setup repeated start.
    //      bus.requestFrom(base, sizeof(T));
    //      if (sizeof(T) == 1) {//expedite common case
    //        return T(bus.read());
    //      } else {
    //        T accumulator;
    //        uint8_t *peeker = reinterpret_cast<uint8_t*>(&accumulator);
    //
    //        for (unsigned bc = sizeof(T); bc-- > 0;) {
    //          *peeker++ = bus.read();
    //        }
    //        return accumulator;
    //      }
    //    }
    //
    //    /** simple block read */
    //    void inp(uint8_t *buffer, unsigned length) {
    //      bus.requestFrom(base, length);
    //      while (length-- > 0) {
    //        *buffer++ = bus.read();
    //      }
    //    }

    /** modify a byte at an address */
    uint8_t update(uint8_t addr, uint8_t ones, uint8_t zeroes) {
      bus.beginTransmission(base);
      bus.write(addr);
      bus.endTransmission(false);//setup repeated start.
      bus.requestFrom(base, 1U);
      uint8_t was = bus.read();

      bus.beginTransmission(base);
      bus.write(addr);
      bus.write((was | ones) & ~zeroes); //#parens required to ensure order of operations. without them the ones&~zeroes combined first, then were or'd into mode, preventing us from clearing bits.
      bus.endTransmission();
    }

};


/**
  usage:
  WireWrapper ww(devaddress);//usually done once in constructor of object

  Transmission msg(ww);//for each message sent.
  msg(register)(value)(anothervalue).go();

  go() is automatically callled when the entity goes out of scope. It is smart about already having been called, and if you wish to abandon a started one call oops() before exiting the scope.

*/

class Transmission {
    bool begun = false;
    bool sent = false;
    WireWrapper &ww;
  public:
    Transmission (WireWrapper &wwa): ww(wwa) {}
    Transmission(const Transmission&) = delete;
    Transmission() = delete;

    /** append more bytes to the message.*/
    template <typename T >  Transmission &operator [](const T &t) {
      if (!begun) {
        ww.bus.beginTransmission(ww.base);
      }
      //output low to high, will eventually want a flag for bigendian targets
      const uint8_t *peeker = reinterpret_cast<const uint8_t*>(&t);
      for (unsigned bc = sizeof(T); bc-- > 0;) {
        ww.bus.write(*peeker++);
      }
    }
    /** actually send it*/
    void go() {
      if (begun && !sent) {
        ww.bus.endTransmission();
        sent = true;
        begun = false;
      }
    }

    /** abandon a transmission */
    void oops() {
      //underlying library doesn't seem to have a concept of quitting. One must hope that the next beginTransmission() takes care of a hanging one.
      begun = false;
      sent = true;
    }

    /** destruction sends the message, if started. @see oops() to prevent that from happening.*/
    ~Transmission() {
      go();
    }

    /** Transmission msg(ww);//for each message sent.
      msg(register)(value)(anothervalue)--(register)(value);*/
    Transmission &operator--(int) {
      go();//finish pending, prepare for new.
      sent = false;
      //begin is false due to go()
    }

};

/** a register within a multi register I2C device. */
template <typename Scalar> class WIred {
    enum {numBytes = sizeof(Scalar)};
    WireWrapper &ww;
    uint8_t selector;
    /** I2C might have different endianess than platform. The default arg below should be a platform derived value */
    bool bigendian;//todo: move this onto ww once we see if any devices have per register endianness (I've seen this with some non-I2C devices)
  public:
    WIred( WireWrapper &ww, uint8_t selector, bool bigendian = false): ww(ww), selector(selector), bigendian(bigendian) {}

    //write to device register
    void operator=(const Scalar &value) {
      ww.bus.beginTransmission(ww.base);
      ww.bus.write(selector);
      const uint8_t *peeker = reinterpret_cast<const uint8_t*>(&value);
      for (unsigned bc = numBytes; bc-- > 0;) {
        ww.bus.write(bigendian ? peeker[bc] : *peeker++);
      }
      ww.bus.endTransmission();
    }

    //read device register, write to select, then repeated start to read values.
    operator Scalar() {
      ww.bus.beginTransmission(ww.base);
      ww.bus.write(selector);
      ww.bus.endTransmission(false);//setup repeated start.
      ww.bus.requestFrom(ww.base, numBytes);
      if (numBytes == 1) {//expedite common case
        return Scalar(ww.bus.read());
      } else {
        Scalar accumulator;
        uint8_t *peeker = reinterpret_cast<uint8_t*>(&accumulator);

        for (unsigned bc = numBytes; bc-- > 0;) {
          uint8_t bite = ww.bus.read();
          if (bigendian) {
            peeker[bc] = bite;
          } else {
            *peeker++ = bite;
          }
        }
        return accumulator;
      }
    }
};
