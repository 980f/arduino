#pragma once

/** a minimal interface to the DK Electronics dual stepper/dual servo board, only exposing dual stepper capability */

namespace SpiDualBridgeBoard {
void start(bool free = false);

void setBridge(bool second, bool x, bool y);

void setBridge(bool second, uint8_t phase) ;

void power(bool second, bool on);

};

class SpiDualStepper {
    const bool second;
  public:
    SpiDualStepper(bool second): second(second) {}

    void setBridge(bool x, bool y) const ;

    //this has the signature to be a Stepper::Interface
    void setBridge(uint8_t phase) const ;

    /** a hard kill tries to lock the rotor, a soft one lets it spin */
    void power(bool on) ;

};
