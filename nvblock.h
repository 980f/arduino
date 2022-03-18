
/** wrapper for putting an object into eeprom at a known location.
    One cannot use this with an object of a class with virtual functions as it stores the vtable, which will change each time the program compiles.
    static Instances are normally const as they can be so should be.
*/
#include <EEPROM.h>
struct Nvblock {
  //address in eeprom
  const unsigned nvbase;
  //numBytes of object, will allocate/use fixed amount of space
  const unsigned allocated;
  //pointer to object;
  byte * const thing;

  unsigned next()const {
    return nvbase + allocated;
  }

  Nvblock(unsigned nvbase, unsigned allocated, byte *thing):
    nvbase(nvbase),
    allocated(allocated),
    thing(thing) {
    //#done.
  }

  Nvblock(Nvblock &&moveit) = default;
  Nvblock(const Nvblock &moveit) = default;

  //  //the following gives 'nameoffirstargument' is not a type.
  //  template<typename Concrete>
  //  Nvblock(Concrete &noVtable, unsigned nvbase): Nvblock(nvbase, sizeof(Concrete), reinterpret_cast<byte *>(&noVtable) {
  //    //#done.
  //  }

  //lean on move semantics to make this nearly as good as a true constructor
  template<typename Concrete> static const Nvblock For(Concrete &noVtable, unsigned nvbase) {
    return Nvblock(nvbase, sizeof(Concrete), reinterpret_cast<byte *>(&noVtable));
  }


  unsigned save()const {
    for (unsigned si = 0; si < allocated; ++ si) {
      EEPROM.write(nvbase + si, thing[si]); //no need to finesse with update() on an esp8266.
    }
    return next();
  }

  unsigned load()const {
    for (unsigned si = 0; si < allocated; ++ si) {
      thing[si] = EEPROM.read(nvbase + si);
    }
    return next();
  }

  template<typename Beware>
  int compare() const {
    byte old[allocated];
    for (unsigned si = 0; si < allocated; ++ si) {
      old[si] = EEPROM.read(nvbase + si);
    }
    /** @returns the usual compare result of the bytes at hand interpreted as the given type*/
    return ( *reinterpret_cast<Beware&>(thing) ) - (*reinterpret_cast<Beware&>(old));
  }

};
