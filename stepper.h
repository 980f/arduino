#pragma once
#include "pinclass.h"
#include "bitbanger.h"
#include "minimath.h"


class Stepper {

  public:
    enum Iface {Uni4, Bip4, Disk3}; //we are slow enough to not want to pay extra for virtual function call.
    Iface iface = Uni4; //
    int step = 0;
    //  unsigned perRevolution=200;
    //  unsigned phase=0;
    //

    void applyPhase(unsigned phase)const;

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
      ++step;
      applyPhase(step);
      //    if((++phase)==perRevolution){
      //      phase=0;
      //    }

    }

    void operator --() {
      --step;
      applyPhase(step);
      //    if((phase==0){
      //      phase=perRevolution;
      //    }
      //  --phase;

    }
};
