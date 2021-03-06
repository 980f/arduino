#include "clockhand.h"

void ClockHand::freeze() {
  freerun = 0;
  if (changed(enabled, false)) {
    ;//might need to delay before starting again.
  }
}


void ClockHand::setTarget(int target) {
  if (changed(this->target, target)) {
    enabled = target != mechanism;
  }
}

void ClockHand::setTime(unsigned hmrs) {
  auto dial = timeperrev(); //size of dial
  setTarget(rate(long(hmrs % dial) * stepperrev , dial)); //protect against stupid input, don't do extra wraps around the face.
}

void ClockHand::setFromMillis(unsigned ms) {
  setTime(rate(ms, msperunit()));
}

bool ClockHand::onTick() {
  if (ticker.perCycle()) {
    if (freerun) {
      mechanism += freerun;     
      return true;
    }
    if (enabled) {
      mechanism += (target - mechanism);//automatic 'signof' under the hood in Stepper class     
      if (target == mechanism) {
        enabled = 0;
      }
      return true;
    } 
    return false;//not enabled, didn't step
  }
  return false;//not needing to step
}

/** update speed, where speed is millis per step */
void ClockHand::upspeed(unsigned newspeed) {
  if (changed(thespeed, newspeed)) {
    ticker.set(thespeed);//this one will stretch a cycle in progress.
//    dbg("\nSpeed:", thespeed);
  }
}

//free run with time set so that one revolution happens per real world cycle of hand
void ClockHand::realtime() {
  upspeed(rate(msperunit()*timeperrev(), stepperrev));
  freerun = 1;
}
