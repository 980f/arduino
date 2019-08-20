#pragma once
#include "hook.h"
#include "minimath.h"//signof
#include <Arduino.h> //some chips require this for 'byte', some do not. 
/** nascent class for stepper interface.
  will be adding accleration and other speed management into this guy over time. */
class Stepper {
  public:
    using Step = int32_t; //24 would be enough for most uses, but 16 too often was a problem and we don't want to template the range.
  private:
    Step step = 0;

  protected:
    void applyPhase(unsigned step) { //had to remove const on this method due to compiler bug, the Hook should not have inherited constness from the object.
      interface(byte(step));
    }

  public:
    // deferring functional vs OOP for one more dev cycle
    /** assign a function which takes a phase value and operates the real device */
    Hook<byte> interface;
    using Interface = Hook<byte>::Pointer;


    /** @returns nominal location */
    operator Step() const { // NOLINT(google-explicit-constructor,hicpp-explicit-conversions)
      return step;
    }

    /** set value as nominal location */
    void operator =(Step location) { // NOLINT(cppcoreguidelines-c-copy-assignment-signature,misc-unconventional-assign-operator)
      //stepOffset = (location ^ step) & 3; //need to deal with this not being zero.
      step = location;
      applyPhase(step);
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

    /** @returns best direction to step to get to target, presumes circular mechanism.  */
    int operator -(Step target){
      return target-step;
    }

};
