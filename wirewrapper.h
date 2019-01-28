#pragma once

#include "Wire.h" //TwoWire class

/** condense use of I2C.
*/


/** symbols for value returned from endTransmission:
*/
enum WireError : uint8_t {
  None = 0,
  BufferOverflow,
  NackedAddress,
  NeackedData,
  Other
};

class WireWrapper {
    friend class Transmission;
    uint8_t base;
    TwoWire &bus;
    WireError lastOp;
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

    void emit(uint8_t bite) {
      bus.write(bite);
    }

    void Start() {
      bus.beginTransmission(base);
    }

    void Start(uint8_t addr) {
      bus.beginTransmission(base);
      emit(addr);
    }

    WireError End(bool stopit = true) {
      return lastOp = bus.endTransmission(stopit);
    }

    /** reads a block into internal buffer */
    unsigned Read(unsigned numBytes) {
      return bus.requestFrom(base, numBytes);
    }

    WireError Write(const uint8_t *peeker, unsigned numBytes, bool reversed = false) {
      Start();
      for (unsigned bc = numBytes; bc-- > 0;) {
        emit(reversed ? peeker[bc] : *peeker++);
      }
      return End();
    }

    WireError Write(uint8_t selector, const uint8_t *peeker, unsigned numBytes, bool reversed = false) {
      Start(selector);
      for (unsigned bc = numBytes; bc-- > 0;) {
        emit(reversed ? peeker[bc] : *peeker++);
      }
      return End();
    }

    /** writes a select then reads a block */
    unsigned ReadFrom(uint8_t selector, unsigned numBytes) {
      Start(selector);
      End(false);//setup repeated start.
      return Read(numBytes);
    }

    /** writes a select then reads a block into a buffer IFFI the whole block was read, else partial read data is still available */
    bool ReadFrom(uint8_t selector, unsigned numBytes, uint8_t *peeker, bool reversed = false) {
      auto actual = ReadFrom(selector, numBytes);
      if (actual != numBytes) {
        return false;
      }

      if (numBytes == 1) {//expedite common case, reversed is moot
        *peeker = next();
        return true;
      }

      for (unsigned bc = numBytes; bc-- > 0;) {
        uint8_t bite = next();
        if (reversed) {
          peeker[bc] = bite;
        } else {
          *peeker++ = bite;
        }
      }
      return true;
    }



    unsigned available() {
      return bus.available();
    }

    uint8_t next() {
      return bus.read();
    }

  public: //now for conveniences
    /** modify a byte at an address */
    uint8_t update(uint8_t addr, uint8_t ones, uint8_t zeroes) {
      Start(addr);
      End(false); //setup repeated start.
      Read(1U);
      uint8_t was = next();

      Start(addr);
      emit((was | ones) & ~zeroes); //#parens required to ensure order of operations. without them the ones&~zeroes combined first, then were or'd into mode, preventing us from clearing bits.
      End();
      return was;
    }

};

////////////////////////////////////////////////////////////////////////////////

/** a register within a multi register I2C device.
  And yes, it is capitalized WEird because I consistently typoed it.*/
template <typename Scalar> class WIred {
    enum {numBytes = sizeof(Scalar)};
    WireWrapper &ww;
    uint8_t selector;
    /** I2C might have different endianess than platform. The default arg below should be a platform derived value */
    bool bigendian;//todo: move this onto ww once we see if any devices have per register endianness (I've seen this with some non-I2C devices)

  public:
    /** each time we read or write we update this value, handy for sequential bit flipping */
    Scalar cached;
    WireError lastOp;
  public:
    WIred( WireWrapper &ww, uint8_t selector, bool bigendian = false): ww(ww), selector(selector), bigendian(bigendian) {}

    /**write to device register */
    void operator=(const Scalar &value) {
      lastOp = ww.Write(selector, reinterpret_cast<const uint8_t*>(&value), numBytes, bigendian);
      if (lastOp == WireError::None) {
        cached = value;
      }
    }

    /** @returns value read from device, check lastOp to see if it happened.  */
    operator Scalar() {
      if (Read(selector, numBytes, reinterpret_cast<uint8_t*>(&cached), bigendian)) {
        lastOp = WireError::None;
      }
      return cached;
    }
};
////////////////////////////////////////////////////////////////////////////////

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
        ww.Start();
      }
      //output low to high, will eventually want a flag for bigendian targets
      const uint8_t *peeker = reinterpret_cast<const uint8_t*>(&t);
      for (unsigned bc = sizeof(T); bc-- > 0;) {
        ww.emit(*peeker++);
      }
    }

    /** actually send it*/
    Transmission & go() {
      if (begun && !sent) {
        ww.End();
        sent = true;
        begun = false;
      }
      return *this;
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
