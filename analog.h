#pragma once  //(C) 2018 Andy Heilveil, github/980F
/** will be adding smoothing to analog inputs so wrap the data now.
    some of the chips have more than 8 or 10 bits so this class 'normalizes' values to 15 bits (avoiding sign bit for now).
    no Arduino will ever have 16 usable bits, that requires very careful board design.
*/

struct AnalogValue {
  unsigned raw;

  AnalogValue(int physical = 0) {
    raw = physical; //todo: shift will be a function of input resolution (10 vs 12) and oversampling rate (8 samples is same as 3 bit shift)
  }

  unsigned operator =(int physical) {
    raw = physical;
    return raw;
  }

  operator unsigned() const {
    return raw;
  }

};

/** makes analog output appear as if a simple variable. This is handy if you want to replace direct use with proxying to another guy, or to disable output but still see what the value would have been.*/
struct AnalogOutput {
    const unsigned pinNumber;
    AnalogOutput(unsigned pinNumber): pinNumber(pinNumber) {
      //#done
    }

    //scaled/smoothed value
    void operator =(AnalogValue av) const {
      analogWrite(pinNumber, av >> 7); //15 bit normalized input, cut it down to 8 msbs of those 15. todo: configure for 10 and maybe 16 bit output.
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
  const unsigned pinNumber;
  AnalogInput(unsigned pinNumber): pinNumber(pinNumber) {
    //#done
  }

  operator AnalogValue() const {
    return AnalogValue(analogRead(pinNumber) << 5) ; //scale up until we get access to the hardware bit that does this.
  }

  //get traditional 10 bit value.
  int raw()const {
    return AnalogValue(analogRead(pinNumber));
  }
};
