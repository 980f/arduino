
/** wrapper for putting an object into eeprom at a known location.
    One cannot use this with an object of a class with virtual functions as it stores the vtable, which will change each time the program compiles.
    static Instances are normally const as they can be so should be.
*/
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
  
#if 0  //need to upgrade compiler so that it can implement template constructor args. This gives quirky error with 4.8.2
  template<typename Concrete> 
  Nvblock(Concrete &noVtable, unsigned nvbase): Nvblock(nvbase, sizeof(Concrete), reinterpret_cast<byte *>(&noVtable) {
    //#done.
  }
#else
  //lean on move semantics to make this nearly as good as a true constructor
  template<typename Concrete> static const Nvblock &For(Concrete &noVtable, unsigned nvbase){
    Nvblock factory(nvbase, sizeof(Concrete), reinterpret_cast<byte *>(&noVtable));
    return factory;
  }  
#endif

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
