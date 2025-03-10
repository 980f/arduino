#pragma once  //(C) 2019, Andy Heilveil, github/980f
#include "stepper.h"

#if 0
//soft millisecond timers are adequate for minutes and hours.
#include "millievent.h"
#else
//increased speed needed for elevator gag
#include "microevent.h"
using TickTimer=MicroStable; 
#endif

/** ClockHand deals with scaling from milliseconds to angle of the clock face, given stepper motor parameters */

class ClockHand {
  public:
    Stepper mechanism;
    //ms per step
    unsigned thespeed = ~0U;
    unsigned pulseWidth = 1; //5: for BYJ 
    unsigned stepperrev = 200;// a common direct drive.
    enum Unit {
      Seconds,
      Minutes,
      Hours,
    };
    Unit unit;

    /** human units to clock angle */
    unsigned timeperrev() const {
      switch (unit) {
        case Seconds: //same as minutes
        case Minutes: return 60;
        case Hours: return 12;
        default: return 0;
      }
    }

    unsigned long msperunit() const {
      switch (unit) {
        case Seconds: return 1000UL;
        case Minutes: return 60 * 1000UL;
        case Hours: return 60 * 60 * 1000UL;
        default: return 0UL;
      }
    }

    //want to be on
    bool enabled = false;
    //think we are on
    bool energized = false; //actually is unknown at init
    /** time to next step */
    TickTimer ticker;
   
    //where we want to be
    int target = ~0U;
    //ignore position, run forever in the given direction
    int freerun = 0;

    void freeze();

    bool needsPower() const {
      return enabled || freerun != 0;
    }

    /** update speed, where speed is millis per step */
    void upspeed(unsigned newspeed);
    
    void setTarget(int target);

    void setTime(unsigned hmrs);

    void setFromMillis(unsigned ms);

    /** declares what the present location is, does not modify target or enable. */
    void setReference(int location) {
      mechanism = location;
    }

    //free run with time set so that one revolution happens per real world cycle of hand
    void realtime() ;

    ClockHand(Unit unit, Stepper::Interface interface): unit(unit) {
      mechanism.interface = interface;
    }

    //placeholder for incomplete code for remote interface
    ClockHand()=default;
    
    /** must be called every millisecond (your best effort, will fail gracefully)
      @returns whether the interface is energized. */
    bool onTick();

    /** @returns nearest integer of time unit for either the current location or the targeted one*/
    unsigned asTime(bool actual=false) const {
      if(stepperrev==0){
    	return 0; //avert lockup on bad data.
      }
      //compute from target, else stepper position
      int step= actual? int(mechanism) : target;
      //ignoring step<0 issues
      int rem= step % stepperrev;
      if(rem<0){//C has strange idea of how modulus should work. clang Linter has bad check on loop variables so switched from prophylatic while to an if.
    	rem += stepperrev;
      }
      //rem/stepperrev is now fraction of cycle
      return rate(timeperrev()*rem,stepperrev);
    }

};
