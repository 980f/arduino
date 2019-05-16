#pragma once
#include "hook.h"

/** nascent class for stepper interface.
  will be adding accleration and other speed management into this guy over time. */
class Stepper {
    int step = 0;
    int stepOffset=0;
    //  unsigned perRevolution=200;
    //  unsigned phase=0;
    
  public:
  /** assign a function which takes a nibble and routes it to your wires.*/
    Hook<byte> interface;
    using Interface=Hook<byte>::Pointer;
    
    operator int() const {
      return step;
    }

		void operator =(int location){
			stepOffset=(location^step)&3;
			step=location;
      applyPhase(step);
		}

    byte applyPhase(unsigned step)const {
      interface(step);
    }

    /** step either forward or back*/
    void operator ()(bool fwd) {
      step += fwd ? 1 : -1;
      applyPhase(step);
    }

    /** if 0 then don't move, else move one step in direction given by sign of @param dir */
    void operator +=(int dir) {
      step += signof(dir);
      applyPhase(step);
    }

    void operator ++() {
      applyPhase(++step);
      //    if((++phase)==perRevolution){
      //      phase=0;
      //    }

    }

    void operator --() {
      applyPhase(--step);
      //    if((phase==0){
      //      phase=perRevolution;
      //    }
      //  --phase;
    }
};
