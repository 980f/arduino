#include "stepper.h"

//todo: get digitalpin class to function so that we can pass pin numbers and have more than one in a system.

const OutputPin<D4> ph0;
const OutputPin<D3> ph1;
const OutputPin<D2> ph2;
const OutputPin<D1> ph3;


static const unsigned grey2[] = {0, 1, 3, 2};//use by bipolar stepper interface

void Stepper::applyPhase(unsigned phase)const {
  switch (iface) {
    case Uni4: {
        unsigned bits = 0x33 >> (phase % 4);
        ph0 = bitFrom(bits, 0);
        ph1 = bitFrom(bits, 1);
        ph2 = bitFrom(bits, 2);
        ph3 = bitFrom(bits, 3);
      }
      break;
    case Bip4: {
        unsigned bits = grey2[phase & 3]; //Gray code count,
        ph0 = bitFrom(bits, 0);
        ph1 = bitFrom(~bits, 0);
        ph2 = bitFrom(bits, 1);
        ph3 = bitFrom(~bits, 1);
      } break;
    case Disk3: { //3 phase motor, commonly extracted from a disk drive.
        unsigned winding = phase % 3; //one of three
        ph0 = winding == 0;
        ph1 = winding == 1;
        ph2 = winding == 2;

      } break;
  }
}
