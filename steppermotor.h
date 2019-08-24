

/** adds velocity to position.  */
struct StepperMotor {
  Stepper pos;
  Stepper::Step target; //might migrate into Stepper
  /// override run and step at a steady rate forever.
  bool freeRun = false;
  /** if and which direction to step*/
  int run = 0;//1,0,-1

  //time between steps
  MicroStable ticker;

  //configuration
  Stepper::Step homeWidth = 15; //todo:eeprom
  MicroStable::Tick homeSpeed = 250000; //todo:eeprom
  BoolishRef *homeSensor;

  bool which;//used for trace messages
  bool edgy;//used to detect edges of homesensor
  Stepper::Step homeOn;
  Stepper::Step homeOff;

  void setTick(MicroStable::Tick perstep) {
    ticker.set(perstep);
  }

  enum Homing { //arranged to count down to zero
    Homed = 0,   //when move to center of region is issued
    BackwardsOff,//capture far edge, so that we can center on sensor for less drift
    BackwardsOn, //capture low to high
    ForwardOff,  //if on when starting must move off
    NotHomed //setup homing run and choose the initial state
  };
  Homing homing = NotHomed;

  //command in progress. might someday make a queue and this would be q not empty.
  bool cip = false;

  void report() const {
    dbg("{W:", which, ",M:", pos, "}");//let host know we are done.
  }

  void moveto(Stepper::Step location,MicroStable::Tick perstep=0){
    if (perstep) {//0== same speed
      setTick(perstep);
    }
    target = location;
    cip= pos != target;
    if(!cip){
      report();
    }
  }

  /** call this when timer has ticked */
  void operator()() {
    if (ticker.perCycle()) {
      bool homeChanged = homeSensor && changed(edgy, *homeSensor);
      if (homeChanged) {
        dbg("W:", which, " sensor:", edgy);
      }
      switch (homing) {
        case Homed://normal motion logic
          if (!freeRun) {
            if (changed(run , pos - target)) {
              if (cip && run == 0) {
                cip=false;//or pick next from queue
                report();
              }
            }
          }
          pos += run;//steps if run !0
          break;

        case NotHomed://set up slow move one way or the other
          freeRun = 0;
          run = 0;
          if (homeSensor) {
            if (edgy) {
              pos = -homeWidth;//a positive pos will be a timeout
              target = 0;
              homing = ForwardOff;
            } else {
              pos = homeWidth;//a negative pos will be a timeout
              target = 0;
              homing = BackwardsOn;
            }
          } else {
            pos = 0;
            target = 0;
            homing = Homed;//told to home but no sensor then just clear positions and proclaim we are there.
          }
          setTick(homeSpeed);
          dbg("Homing started:", target, " @", homing, " width:", homeWidth);
          break;

        case ForwardOff://HM:3
          if (!edgy) {
            dbg("Homing backed off sensor, at ", pos);
            pos = homeWidth;//a negative pos will be a timeout
            target = 0;
            homing = BackwardsOn;
          } else {
            pos += 1;
          }
          break;

        case BackwardsOn://HM:2
          if (edgy) {
            dbg("Homing found on edge at ", pos);
            pos = homeWidth;//so that a negative pos means we should give up.
            target = 0;
            homing = BackwardsOff;
          } else {
            pos += -1;//todo: -= operator on pos.
          }
          break;

        case BackwardsOff://HM:1
          if (!edgy) {
            dbg("Homing found off edge at ", pos);
            pos = (homeWidth + pos) / 2;
            target = 0;
            homing = Homed;
            stats(&dbg);
          } else {
            pos += -1;
          }
          break;
      }
    }
  }

  //active state report, formatted as lax JSON (no quotes except for escaping framing)
  void stats(ChainPrinter *adbg) {
    if (adbg) {
      (*adbg)("{W:", which, ", T:", target, ", P:", int(pos), ", FR:", freeRun ? run : 0, ", HM:", homing, ", tick:", ticker.duration, '}');
    }
  }

  /** stop where it is. */
  void freeze() {
    homing = Homed;//else failed homing will keep it running.
    freeRun = false;
    target = pos;
  }

  /** not actually there until the step has had time to settle down. pessimistically that is when the next step would occur if we were still moving.*/
  bool there() const {
    return run == 0 && target == pos;
  }

  bool atIndex() {
    freeze();
    pos = 0;
  }

  void start(bool second, Stepper::Interface iface, BoolishRef *homer) {
    which = second;
    pos.interface = iface;
    if (changed(homeSensor, homer)) {
      if (homeSensor != nullptr) {
        homing = NotHomed;
      } else {
        homing = Homed;
      }
    }
    setTick(homeSpeed);
  }
};

