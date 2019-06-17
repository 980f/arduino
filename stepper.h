#pragma once
#include "hook.h"
#include "minimath.h"//signof
#include <Arduino.h> //some chips require this for 'byte', some do not. 
/** nascent class for stepper interface.
  will be adding accleration and other speed management into this guy over time. */
class Stepper {
    int step = 0;
//    int stepOffset = 0;
//    bool energized = false;
   
  public:
    // deferring functional vs OOP for one more dev cycle
    /** assign a function which takes a phase value and operates the real device */
    Hook<byte> interface;
    using Interface = Hook<byte>::Pointer;
   
//    Stepper(Mechanism &interface): interface(interface) {}

    /** @returns nominal location */
    operator int() const {
      return step;
    }

    /** set value as nominal location */
    void operator =(int location) {
      //stepOffset = (location ^ step) & 3; //need to deal with this not being zero.
      step = location;
      applyPhase(step);
    }

    void applyPhase(unsigned step) { //had to remove const on this method due to compiler bug, the Hook should not have inherited constness from the object.
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
