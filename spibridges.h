#pragma once

/** a minimal interface to the DK Electronics dual stepper/dual servo board, only exposing dual stepper capability */

namespace SpiDualBridgeBoard {
  static void start(bool free = false);

  static void setBridge(bool second, bool x, bool y);

  static void setBridge(bool second, uint8_t phase) ;

  static void power(bool second,bool on);

};

class SpiDualStepper {
  const bool second;
public:
  SpiDualStepper(bool second):second(second){}

  static void start(bool free = false);//talks to the soliton to get it ready

  void setBridge(bool x, bool y) const ;

  //this has the signature to be a Stepper::Interface
  void setBridge(uint8_t phase) const ;

  /** a hard kill tries to lock the rotor, a soft one lets it spin */
  void power(bool on) ;

};
