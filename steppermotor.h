#include "char.h"
#include "microevent.h"
#include "millievent.h"

#ifdef MotorDebug
#include "chainprinter.h"
ChainPrinter mdbg(MotorDebug, true);
#else
#define mdbg(...)
#endif


/** adds velocity to position.  */
struct StepperMotor {
  using Tick = MicroStable::Tick;
  Stepper pos;
  Stepper::Step target; //might migrate into Stepper
  /// override run and step at a steady rate forever.
  bool freeRun = false;
  /** if and which direction to step*/
  int run = 0;//1,0,-1
  /** stopped is true while we are in low power state. */
  bool stopped = false;

  //time between steps
  MicroStable ticker;
  //time to lower pwer so motor doesn't burn.
  MonoStable powersave();
  ///////////////////////////////
  //this entity computes step width given a starting and a change per step.
  //it does not apply it to the hardware.
  struct GasPedal {
    //configuration
    Tick start = 0;
    Tick accel = 0; //we trust this is less than 'start'
    //desired speed
    Tick cruise = 0; //formerly the one and only speed, trust to be less than or equal to start.

    /** velocity mode (aka freeRun) computation, ignores steps remaining in motion */
    Tick operator()(Tick present) const {
      if (present > start) {
        return start;//todo:M start is also the slowest we allow, this is probably a bad idea for some users.
      }
      if (present >= (cruise + accel)) {//if can subtract without exceding desired then do so
        return present - accel;
      }
      return cruise;//target
    }

    /** @return time for next step given number of steps remaining, will do the abs value locally */
    Tick operator()(Stepper::Step remaining, Tick present) const {
      if (0 == signabs(remaining)) {
        return ~0;//kill timer, stop motion if no steps left
      }
      //the following merits caching:
      unsigned ramp = quanta(present - cruise, accel); //change in speed desired / number of steps to do it in
      if (remaining < ramp) {//if can't make it to full speed before stopping
        Tick newrate = start - accel * ramp;//max speed allowed per step
        //todo:0 we don't want the ramp down to ramp up. need to add logic for that.
        return newrate;
      }
      return operator()(present);
    }

    void configure(decltype(accel) a, decltype(start) p) {
      accel = a; //#NB: a zero acceleration is allowed.
      if (p != 0) {//# but a zero speed implies 'retain present'
        start = p;
      }
    }
  } g;
  /////////////////////////////////////////////////////////
  //configuration, class default should be non functional.
  struct Wheel {
    Stepper::Step rev = 200; //pretty much the largest at hand, although there is a 2k/rev on our supplies it moves slowly.
    Stepper::Step width = ~0; //0 is non functional, don't try to home, ignore sensor.
    void configure(decltype(width) a, decltype(rev) p) {
      if (p) {
        rev = p;
      }
      if (a) {
        width = a;
      }
    }
    bool isValid() const {
      return width > 0 && width < rev;
    }
  } h;

  //physical config:
  bool which;//used for trace messages
  BoolishRef *homeSensor;
  BoolishRef *powerControl;

  void setCruise(Tick perstep) {
    if (changed(g.cruise, perstep)) {    	
      ticker.set(perstep,false);//adjust present cycle, do not restart the timer.
    }
  }

  enum Homing { //arranged to count down to zero
    Homed = 0,   //when move to center of region is issued
    BackwardsOff,//capture far edge, so that we can center on sensor for less drift
    BackwardsOn, //capture low to high
    ForwardOff,  //if on when starting must move off
    NotHomed //setup homing run and choose the initial state
  };
  Homing homing = NotHomed;
  //supports state machine
  bool edgy;//used to detect edges of homesensor

  /** command in progress. might someday make a queue and this would be q not empty.
      used to send a report when the target is reached. */
  bool cip = false;

  void report() const {
    dbg("{W:", which, ",M:", pos, '}');//let host know we are done.
  }

  void moveto(Stepper::Step location,  Tick perstep = 0) {
    if (perstep) {//0== same speed
      setCruise(perstep);
    }
    target = location;
    cip = pos != target;
    if (!cip) {
      report();
    }
    if (!run) {
      operator()();
    }
  }

  /** freerun mode, aka velocity mode*/
  void Run(bool forward) {
    run = forward ? 1 : -1;
    freeRun = true;
  }

  /** call this when timer has ticked */
  bool operator()() {
    if (ticker.perCycle()) {//using perCycle instead of isDone to keep the timer going on all paths through this function.
      bool homeChanged = homeSensor && changed(edgy, *homeSensor);
      if (homeChanged) {
        mdbg("W:", which, " sensor:", edgy);
      }
      switch (homing) {
        case Homed://normal motion logic
          if (freeRun) {
            ticker = g(ticker.duration);
          } else {
            if (changed(run , pos - target)) {
              if (cip && run == 0) {
                cip = false; //or pick next from queue
                report();
              }
            }
            ticker = g(run, ticker.duration);
          }
          pos += run;//steps if run !0
          break;//

        case NotHomed://set up slow move one way or the other
          freeRun = 0;
          run = 0;
          if (homeSensor) {
            if (edgy) {
              pos = -h.width;//a positive pos will be a timeout
              target = 0;
              homing = ForwardOff;
            } else {
              pos = h.rev;//a negative pos will be a timeout
              target = 0;
              homing = BackwardsOn;
            }
          } else {
            pos = 0;
            target = 0;
            homing = Homed;//told to home but no sensor then just clear positions and proclaim we are there.
          }
          setCruise(g.start);
          mdbg("Homing started:", target, " @", homing, " width:", h.width);
          break;

        case ForwardOff://HM:1
          if (!edgy) {
            mdbg("Homing backed off sensor, at ", pos);
            pos = h.width;//a negative pos will be a timeout
            target = 0;
            homing = BackwardsOn;
          } else {
            pos += 1;
          }
          break;

        case BackwardsOn://HM:2
          if (edgy) {
            mdbg("Homing found on edge at ", pos);
            pos = h.width;//so that a negative pos means we should give up.
            target = 0;
            homing = BackwardsOff;
          } else {
            pos += -1;//todo: -= operator on pos.
          }
          break;

        case BackwardsOff://HM:3
          if (!edgy) {
            mdbg("Homing found off edge at ", pos);
            pos = (h.width + pos) / 2;
            target = 0;
            homing = Homed;
            stats(&dbg);
          } else {
            pos += -1;
          }
          break;
      }
      return true;//now would be a good time for a flying change.
    } else {
      return false;
    }
  }

  //active state report, formatted as lax JSON (no quotes except for escaping framing)
  void stats(ChainPrinter *adbg) {
    if (adbg) {
      (*adbg)("{W:", which, ", T:", target, ", P:", pos, ", FR:", freeRun ? run : 0, ", HM:", homing, ", tick:", ticker.duration, '}');
    }
  }

  Char runcode() const {
    Char code('x');
    if (run < 0) {
      code = 'r';
    } else if (run) {
      code = 'f';
    }
    if (which == 0) {
      code.toUpper();
    }
    return code;
  }

  /** stop where it is. */
  void freeze() {
    homing = Homed;//else failed homing will keep it running.
    freeRun = false;
    target = pos;
  }

  void power(bool on) {
    if (powerControl != nullptr) {
      *powerControl = on;
    }
  }

  /** not actually there until the step has had time to settle down. pessimistically that is when the next step would occur if we were still moving.*/
  bool there() const {
    return run == 0 && target == pos;
  }

  void atIndex() {
    freeze();
    pos = 0;
  }

  /** start homing */
  void home() {
    homing = (homeSensor != nullptr && h.isValid()) ? NotHomed : Homed;
  }

  void start(bool second, Stepper::Interface iface, BoolishRef *homer, BoolishRef *powerGizmo) {
    which = second;
    pos.interface = iface;
    powerControl = powerGizmo; //no action needed on connect
    homeSensor = homer; //4dbg
    home();
    setCruise(g.start);
    ticker=g.start;//without this the logic doesn't run, the ticker powers up disabled.
  }
};
