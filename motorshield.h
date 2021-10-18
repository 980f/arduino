/** seedstudio V1.1 motor shield
		L298 based dual H-bridge as independent drives (or paired driver)
*/

#include "pinclass.h"

/** there are four half-H's as two pairs due to shared enable.
  This class is for one pair of half-H's.

  It doesn't allow 100% of the options allowed by the hardware, only one brake mode is exposed, the other is replaced with diddling the enable pin.
*/
struct L298Bridge {
  const DigitalOutput onehalf;
  const DigitalOutput otherhalf;
  const DigitalOutput enable;
  L298Bridge (PinNumberType one, PinNumberType other, PinNumberType enabler, bool invertall = false)
    : onehalf(one, !invertall)
    , otherhalf(other, !invertall)
    , enable(enabler, !invertall)
  {
  }

  enum Code {
    Off = 0,
    Forward,
    Backward,
    Hold
  };


  /** 1 goes one way, 2 goes the other way, 0 disables, 3 puts on the breaks.*/
  void operator =(Code code) {
    onehalf = !!(code & 1);
    otherhalf = !!(code & 2);
    enable = code != 0;
  }

  //for using soft pwm to control power directly assign to enable member.


  /** @returns nominal direction, ignoring whether it is enabled so that enable can be used for power level control via PWM'ing it */
  operator Code () const {
    if (onehalf) {
      return otherhalf ? Hold : Forward;
    } else {
      return otherhalf ? Backward : Off;
    }
  }

};


/** hard wired pin assignments and polarity.
    Note: 9 and 10 were chosen by Seeedstudio as they are PWM outputs.
*/
struct SeeedStudioMotorShield {
  const L298Bridge one{8, 11, 9};
  const L298Bridge two{12, 13, 10};

  /** operate both in tandem */
  void operator =(unsigned code) {
    one = code;
    two = code;
  }

  /** @returns nominal direction, ignoring whether it is enabled so that enable can be used for power level control via PWM'ing it */
  L298Bridge::Code state() const {
    return one.operator Code();
  }
};
