#pragma once //(C)2019 Andy Heilveil, github/980f
/**
  init a program that allows tweaks by text input.
  It supports a block of const string, which is stored in the program body, and a block of EEPROM.
  One can load from the code, or from the eeprom, and one can copy the rom to the eeprom,
  but saving to eeprom of present state requires an external agent.
*/
#include "eepointer.h"
#include "rompointer.h"

using KeyHandler = void (*)(byte key);

class Initer {
  public://made 'em all const, so read em and weep
    RomAddr initdata;
    KeyHandler doKey;
    /** offset to block in eeprom, used to reserve some space for other uses (although why would you?)*/
    const uint16_t start;
  public:
    Initer(RomAddr initdata, KeyHandler doKey, uint16_t start = 0): initdata(initdata), doKey(doKey), start(start) {}

    /** restore developer settings */
    unsigned restore(bool andsave = true) {
      RomPointer rp(initdata);
      EEPointer eep =  EEPrinter(start); //create even if we aren't going to use, creation is cheap
      while (byte c = *rp++) {
        if (doKey) {//compiler silently converter fn pointer into unsigned.
          (*doKey)(c);
        }
        if (andsave) {
          if (eep) {
            *eep++ = c;
          }
        }
      }
      if (andsave && eep) {//really should null to end of eeprom at least once. Perhaps on invocation of restore(true).
        *eep = 0;//terminating null
      }
      return (rp - initdata);
    }

    /** Print'ing to this writes to EEProm */
    EEPrinter saver() {
      return EEPrinter(start);
    }

    /** EEprom content passed byte at a time to supplied keyhandler.
      Ignores chars beyond 'normal' ascii. */
    unsigned load() {
      unsigned actuals = 0;
      EEPointer eep = EEPointer(start);
      while (eep) {//don't run off the end of existence
        byte c = *eep++;
        if (c > '~') { //most likely uninit eeprom.
          continue;
        }
        if (c) {
          ++actuals;
          (*doKey)(c);
        } else {
          break;
        }
      }
      return actuals;
    }

};
