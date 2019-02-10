#pragma once  //(C) 2018 Andy Heilveil, github/980F
/** will be adding smoothing to analog inputs so wrap the data now.
    some of the chips have more than 8 or 10 bits so this class 'normalizes' values to 15 bits (avoiding sign bit for now).
    no Arduino will ever have 16 usable bits, that requires very careful board design.
*/

class AnalogValue {
  using F15=uint16_t ;//1.15 fraction
  protected:
    F15 raw;
  public:
    static const F15 Min = (0);
    static const F15 Half= (0x4000);
    static const F15 Max = (0x7FFF);

    AnalogValue(int physical = 0) {
      raw = physical; //todo: shift will be a function of input resolution (10 vs 12) and oversampling rate (8 samples is same as 3 bit shift)
    }

    unsigned operator =(int physical) {
      raw = physical;
      return raw;
    }

    /** @returns raw value, a value like it.
      operator int() created construction difficulties for objects which take an int construction arg as well as have an operator =(AnalogValue) */
    unsigned operator ~() const {
      return raw;
    }

    /** @returns value on other side of midpoint */
    unsigned operator -() const {
      return (Max + Min) - raw;
    }

};



/** makes analog output appear as if a simple variable. This is handy if you want to replace direct use with proxying to another guy, or to disable output but still see what the value would have been.*/
struct AnalogOutput {
    const short pinNumber;
    AnalogOutput(short pinNumber): pinNumber(pinNumber) {
      //#done
    }

    //scaled/smoothed value
    void operator =(AnalogValue av) const {
      analogWrite(pinNumber, ~av >> 7); //15 bit normalized input, cut it down to 8 msbs of those 15. todo: configure for 10 and maybe 16 bit output.
    }

    //traditional arduino value
    void operator =(int raw) const {
      analogWrite(pinNumber, raw);
    }

  private:
    void operator =(AnalogOutput &other) = delete; //to stifle compiler saying that it did this on it own, good for it :P
};

/** makes analog input appear as if a simple variable. This is handy if you want to replace direct use with proxying to another guy, or to feed logic from a different source.*/
struct AnalogInput {
  const short pinNumber;
  AnalogInput(short pinNumber): pinNumber(pinNumber) {
    //#done
  }

  operator AnalogValue() const {
    return AnalogValue(analogRead(pinNumber) << 5) ; //scale up until we get access to the hardware bit that does this.
  }

  //get traditional 10 bit value.
  int raw() const {
    return int(~AnalogValue(analogRead(pinNumber)));
  }
};

/* RC, aka exponential decay, averaging. */
struct SmoothedAnalogValue: public AnalogValue {
  unsigned shift;//power of two scaling is faster than multiply. Someday we will import the 16*16/16 code and make this class better.

  SmoothedAnalogValue(int physical = 0, unsigned shift = 5) :
    AnalogValue(physical),
    shift(shift) {
    //#done
  }


  /** adding 2^-shift * input and subtracting out 2^-shift * present value, works like an RC filter with a decay of 2^-shift each 'clock'.
       A moving average behaves better, but this is almost as good and is really cheap in memory usage.
  */
  unsigned operator =(int physical) {
    raw += (physical - raw) >> shift;
    return raw;
  }

};
