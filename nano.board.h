#pragma once //(C)2021 Andy Heilveil, github/980F

/**
This was copied from promicro before it was realize that the only differences are pinouts, not any of the core functionality.
There will be a base class of AVR16 and then some simple extensions which represent which components are present on each board.
*/

#include "bitbanger.h"
#include "chainprinter.h"  //for debug stream
//board model, includes chip type.

//  TIMER1A,    /* 9 */
//  TIMER1B,    /* 10 */
class AVR328P {
  public:
    enum {//processor constants
      unsigned_bits = 16, //AVR does not provide much of C++ library :(

    };

    struct Port {
      byte pins;
      byte ddir;
      byte bits;
    };

    struct PinReference {
      static Port &forLetter(char letter) {
        switch (letter & 7) { //cheap way to do case insensitive, but only is valid for valid values.
          default: return *reinterpret_cast<Port*>(PORTC);//all problems tossed in with least useful port to appease compiler.
          case 2: return *reinterpret_cast<Port*>(PORTB);
          case 3: return *reinterpret_cast<Port*>(PORTC);
          case 4: return *reinterpret_cast<Port*>(PORTD);
          case 5: return *reinterpret_cast<Port*>(PORTE);
          case 6: return *reinterpret_cast<Port*>(PORTF);
        }
      }
      Port &port;
      const unsigned shift;

      PinReference(char letter, unsigned shift, bool out): port(forLetter(letter)), shift(shift) {
        drive(out);//don't need to wait for setup().
      }

			/** @returns the actual pin value, which might not be what you output */
      operator bool () const {
        return (port.pins & (1 << shift)) != 0;
      }

      bool operator =(bool setit) {
        if (setit) {
          port.bits |= (1 << shift);
        } else {
          port.bits &= ~(1 << shift);
        }
        return setit != 0; //canonical
      }

      /** set data direction */
      void drive(bool out) {
        if (out) {
          port.ddir |= (1 << shift);
        } else {
          port.ddir &= ~(1 << shift);
          port.bits |= (1 << shift); //enable pullups all around, the device isn't flexible enough to make this a choice.
        }
      }
    };
/// so far same as 32u4 ...

    class T1Control {
      public://here through resolution() are part of picking the precalar. Most systems have 16MHz as the input to this but that is configurable via the PLL stuff (not yet encoded).
        enum  CS {
          Halted = 0,
          By1 = 1,
          By8 = 2,
          By64 = 3,
          By256 = 4,
          By1K = 5,
          ByFalling = 6,
          ByRising = 7
        };

        T1Control() {}

        static constexpr unsigned divisors[ByRising + 1] = {0, 1, 8, 64, 256, 1024, 1, 1};

        static unsigned resolution(CS cs) {
          return divisors[cs & 7];
        }

        /** ticks must be at least 3, typically as large as you can make it for a given CS.
            example: 50Hz@16MHz: 320k ticks, use divide By8 and 40000.
            Note that we are this time ignoring the possibility of dicking with the shared prescalar.
        */
        void setPwmBase(unsigned ticks, CS cs)const { //pwm cycle time
          TCCR1A = 0;//disable present settings to prevent evanescent weirdness
          TCCR1B = 0;// ditto
          TCNT1  = 0;//to make startup repeatable
          ICR1 = ticks;
          // turn on CTC mode with ICR as period definer (WGM1.3:0 = b1110)
          TCCR1A = 2;//WGM1:0
          TCCR1B = (3 << 3) | cs; //3<<3 is WGM3:2. 3 lsbs are divisor select.
        }

        template <unsigned which> class PwmBit {
            enum {
              shift = 8 - (which * 2), //1->6, 2->4, 3->2
              mask = 3 << shift,//two control bits for each
              Dnum = 8 + which, //Arduino digital label for output pin, used by someone somewhen to make pin be an output.
            };
            //set output pin mode, RTFM or see where this is called.
            void configure(unsigned twobits) const {
              mergeInto(TCCR1A, (twobits << shift), mask); //todo: bring out the bit field class.
            }

          public:
            //toggle mode has the bit change state when the compare occurs. One uses 'force on' 'force off' to set the polarity before FIRST cycle, only really useful in up/down mode.
            void toggle()const {
              configure(1);
            }

            /** nominal turn PWM off for this channel, actually just disconnects from timer, remember to set bit to desired state via DIO before calling this. */
            void off() const {
              configure(0);
            }

            /** set delay from cycle begin to where output changes, @param invertit sets polarity of signal, value is that of early part of cycle */
            void setDuty(unsigned ticks, bool invertit = 0)const {
              if (ticks) { //at least 1 enforced here
                configure(2 + invertit);
                //  address order: TCCR, ICR, OCRA, OCRB, OCRC
                (&ICR1)[which] = ticks - 1;
              } else {     //turn it off.
                off();
              }
            }

            /** @returns value of given param after possible changing it to be less than or equal to the cycle max.*/
            static unsigned clip(unsigned &ticks) {
              if (ticks > ICR1) {
                ticks = ICR1;
              }
              return ticks;
            }

        };

        PwmBit<1> pwmA;//PB5, D9
        PwmBit<2> pwmB;//PB6, D10
        // not available off chip       PwmBit<3> pwmC;
        void showstate(ChainPrinter &dbg) {
          dbg("\nT1\tCRA: ", TCCR1A, "\tCRB:", TCCR1B, "\tOA:", OCR1A, "\tOB:", OCR1B, "\tICR:", ICR1);
        }


        /** @returns the prescale setting required for the number of ticks. Will be Halted if out of range.*/
        static constexpr CS prescaleRequired(long ticks) {
          //older compiler makes me pack this into nested ternaries. the thing added to 16 is the power of two that the next line will return
          return  (ticks >= (1L << (16 + 10))) ? Halted : //out of range
                  (ticks >= (1L << (16 + 8))) ? By1K :
                  (ticks >= (1L << (16 + 6))) ? By256 :
                  (ticks >= (1L << (16 + 3))) ? By64 :
                  (ticks >= (1L << (16 + 0))) ? By8 :
                  By1;
        }

    } T1;
};

struct NanoAVR: public AVR328P {
  //todo: get digitalpin class to function so that we can pass pin numbers.
  const OutputPin<LED_BUILTIN_RX> led1;
  //    const OutputPin<24> led0;//nice proc-micro graphic is wrong. arduino dox are wrong.

  //    PinReference
  bool ledd5;
  NanoAVR()
  //    :ledd5('D',5,1)
  {

  }
};
