#pragma once
#include "hook.h"
#include "minimath.h"//signof
#include <Arduino.h> //some chips require this for 'byte', some do not. 
/** nascent class for stepper interface.
  will be adding accleration and other speed management into this guy over time. */
class Stepper {
    int step = 0;
    int stepOffset = 0;
    bool energized=false;
    //  unsigned perRevolution=200;
    //  unsigned phase=0;

  public:
    // and once again functional programming proves to be less able than OOP. We would have to config two foreign functions which most likely share state, or use the 40 year old technique of magic values.
    //  /** assign a function which takes a nibble and routes it to your wires.*/
    //    Hook<byte> interface;
    //    using Interface=Hook<byte>::Pointer;
    struct Mechanism {
      operator ()(byte step) = 0;
      void power(bool beOn) = 0;
    };
    Mechanism &interface;

    Stepper(Mechanism &interface): interface(interface) {}

    /** @returns nominal location */
    operator int() const {
      return step;
    }

    /** set value as nominal location */
    void operator =(int location) {
      stepOffset = (location ^ step) & 3; //need to deal with this not being zero.
      step = location;
      applyPhase(step);
    }

    void applyPhase(unsigned step) { //had to remove const on this method due to compiler bug, the Hook should not have inherited constness from the object.
    	if(changed(energized,true)){
    		interface.power(energized);
    	}
      interface(byte(step));
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
