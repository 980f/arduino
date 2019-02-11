#pragma once //(C)2019 Andy Heilveil, github/980f
/**
init a program that allows tweaks by text input.
It supports a block of const string, which is stored in the program body, and a block of EEPROM.
One can load from the code, or from the eeprom, and one can copy the rom to the eeprom,
but saving to eeprom of present state requires an external agent.
*/
#include "eepointer.h"
#include "rompointer.h"

using KeyHandler = void (*)(char key);

class Initer {
  public://made 'em all const, so read em and weep
    RomAddr initdata;
    KeyHandler doKey;
    /** offset to block in eeprom, used to reserve some space for other uses (although why would you?)*/
    const uint16_t start;
  public:
    Initer(RomAddr initdata, KeyHandler doKey, uint16_t start = 0): initdata(initdata), doKey(doKey), start(start) {}

    void report(uint16_t bytecount) {
      Console(FF("Init block is "), bytecount, " bytes");
    }

    /** restore developer settings */
    void restore(bool andsave = true) {    	
      Console(FF("Init restore: "));//, reinterpret_cast<const __FlashStringHelper *>(initdata));
      RomPointer rp(initdata);
      EEPointer eep = saver(); //create even if we aren't going to use, creation is cheap
      while (char c = *rp++) {
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
      report(rp - initdata);
    }

    /** generates commands to recreate the present state*/
    EEPointer saver() {
      return EEPointer(start);
    }

    void load() {
      Console(FF("Configuring from eeprom: "));
      unsigned actuals=0;
      EEPointer eep = saver();//guarantee overlap
      while (eep) {//don't run off the end of existence
        char c = *eep++;
        if (c < 0 || c==255 || c>'~') {//uninit eeprom. If we ever get the eeprom to init correctly we will make this a terminator.
          continue;
        }
        if (c) {
        	++actuals;
          (*doKey)(c);
        } else {
          break;
        }
      }
      report(actuals);
    }

};
