#pragma once  //(C) 2018 Andy Heilveil, github/980F
/** will be adding smoothing to analog inputs so wrap the data now.
    some of the chips have more than 8 or 10 bits so this class 'normalizes' values to 15 bits (avoiding sign bit for now).
    no Arduino will ever have 16 usable bits, that requires very careful board design.
    
    todo: see analogReadResolution and if it exists for the board use it instead of this class, or use it in this class via #ifdef'ing the bulk of it to triviality.
*/

class AnalogValue: public Printable {
    using F15 = uint16_t ; //1.15 fraction
  protected:
    F15 raw;
  public:
    //this are named both because that is generally good and because we anticipate enhancing the class to have an asymmetric range when we add signedness to it.
    static const F15 Min = (0);
    static const F15 Half = (0x4000);
    static const F15 Max = (0x7FFF);

    AnalogValue(unsigned physical, unsigned bits = 15) {
      raw = physical << (15 - bits);
    }

    AnalogValue(): raw(0) {}

    AnalogValue(AnalogValue &&other) = default;
    AnalogValue(const AnalogValue &other) = default;
    AnalogValue & operator =(const AnalogValue &other) = default;

    //    unsigned operator =(unsigned physical) {
    //      raw = physical;
    //      return raw;
    //    }

    bool operator ==(AnalogValue &&other) const {
      return raw == other.raw;
    }

    bool operator ==(AnalogValue other) const {
      return raw == other.raw;
    }


    bool operator !=(AnalogValue &&other) const {
      return raw != other.raw;
    }


    bool operator !=(AnalogValue other) const {
      return raw != other.raw;
    }


    
    /** @returns raw value, a value like it.
      operator int() created construction difficulties for objects which take an int construction arg as well as have an operator =(AnalogValue) */
    unsigned bits() const {
      return raw;
    }


    /**15 bit version of typical '~' functioning. 
     * created for linearmap reversed scaling.
     */
    unsigned operator ~() const {
      return 32767U-raw;
    }
  
    unsigned operator /(unsigned divisor)const{
      return raw/divisor;
    }

    /** @returns value on other side of midpoint */
    unsigned operator -() const {
      return (Max + Min) - raw;
    }

    size_t printTo(Print& p) const {
      return p.print(raw);
    }
};



/** makes analog output appear as if a simple variable. This is handy if you want to replace direct use with proxying to another guy, or to disable output but still see what the value would have been.

todo: template number of bits in device  (template <unsigned numbits>) where the value for numbits comes from some processor defines \ 
 or use analogWriteResolution() if that function exists and set it to 15 and then remove the shift in the operator=() method here.
*/

struct AnalogOutput {
    const short pinNumber;
    AnalogOutput(short pinNumber): pinNumber(pinNumber) {
      //#done
    }

    //scaled/smoothed value
    void operator =(AnalogValue av) const {
      analogWrite(pinNumber, ~av >> (15 - 8)); //15 bit normalized input, cut it down to 8 msbs of those 15.
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
    return AnalogValue(analogRead(pinNumber) , 10) ; //scale up until we get access to the hardware bit that does this.
  }

  //get traditional 10 bit value.
  int raw() const {
    return analogRead(pinNumber);
  }
};

///* RC, aka exponential decay, averaging. */
//the following is an import from another 980f library, but not yet tested for utility or correctness.
//struct SmoothedAnalogValue: public AnalogValue {
//  unsigned shift;//power of two scaling is faster than multiply. Someday we will import the 16*16/16 code and make this class better.
//
//  SmoothedAnalogValue(int physical = 0, unsigned shift = 5) :
//    AnalogValue(physical),
//    shift(shift) {
//    //#done
//  }
//
//
//  /** adding 2^-shift * input and subtracting out 2^-shift * present value, works like an RC filter with a decay of 2^-shift each 'clock'.
//       A moving average behaves better, but this is almost as good and is really cheap in memory usage.
//  */
//  unsigned operator =(int physical) {
//    raw += (physical - raw) >> shift;
//    return raw;
//  }
//
//};
