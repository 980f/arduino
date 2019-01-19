#pragma once

#include "Wire.h" //TwoWire class

/** Hide proper use of I2C.
*/
class WireWrapper {
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

    template <typename T > void outp(T t) {
      //output low to high, will eventually want a flag for bigendian targets
      uint8_t *peeker = reinterpret_cast<uint8_t*>(&t);
      for (unsigned bc = sizeof(T); bc-- > 0;) {
        bus.write(*peeker++);
      }
    }


    template <typename First, typename ... Args> void more(const First &first, const Args & ... args) {
      outp(first);
      if (sizeof...(args) > 0) {
        more(args...);
      }
    }

    template <typename ... Args> void more() {

    }


    template <typename ... Args> void send(Args ... args) {
      bus.beginTransmission(base);
      more(args...);
      bus.endTransmission();
    }


    /** read a device register.
      until we manage to do the painful coding for varargs use packed structs to get a multiple return. */
    template <typename T > T inp(uint8_t addr) {
      bus.beginTransmission(base);
      bus.write(addr);
      bus.endTransmission();//there really should be a 'repeated start' capability.
      bus.requestFrom(base, sizeof(T));
      if (sizeof(T) == 1) {//expedite common case
        return bus.read();//may need to do some type casting now and then.
      } else {
        T accumulator;
        uint8_t *peeker = reinterpret_cast<uint8_t*>(&accumulator);

        for (unsigned bc = sizeof(T); bc-- > 0;) {
          *peeker++ = bus.read();
        }
        return accumulator;
      }
    }

    /** simple block read */
    void inp(uint8_t *buffer, unsigned length) {
      bus.requestFrom(base, length);
      while (length-- > 0) {
        *buffer++ = bus.read();
      }
    }


};
