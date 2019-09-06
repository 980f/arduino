#include "wirewrapper.h"
#include "cheaptricks.h" //changed

unsigned WireWrapper::bus_kHz = 0; //zero init used for lazy begin, do not set to a default kHz.

void WireWrapper::Start() {
  if (bus_kHz == 0) {//lazy begin
    begin();
  }
  if (changed(bus_kHz, kHz)) { //lazy setclock.
    bus.setClock(kHz * 1000); //TwoWire and utility/twi.c need a bunch of work to allow dynamic speed switching.
  }
  bus.beginTransmission(base);
}

/** send a block of data, with @param reversed determining byte order. default is what is natural for your processor, so you should probably never use the default! */
WireError WireWrapper::Write(const uint8_t *peeker, unsigned numBytes, bool reversed) {
  Start();
  if (numBytes == 1) {//expedite common case, reversed is moot
    emit(*peeker);
  } else {
    for (unsigned bc = numBytes; bc-- > 0;) {
      emit(reversed ? peeker[bc] : *peeker++);
    }
  }
  return End();
}

/** send a block of data to an 8 bit subsystem of your device.*/
WireError WireWrapper::Write(uint8_t selector, const uint8_t *peeker, unsigned numBytes, bool reversed ) {
  Start(selector);
  for (unsigned bc = numBytes; bc-- > 0;) {
    emit(reversed ? peeker[bc] : *peeker++);
  }
  return End();
}

/** writes a select then reads a block */
unsigned WireWrapper::ReadFrom(uint8_t selector, unsigned numBytes) {
  Start(selector);
  End(false);//setup repeated start.
  return Read(numBytes);
}

/** writes a select then reads a block into a buffer IFFI the whole block was read, else partial read data is still available */
bool WireWrapper::ReadFrom(uint8_t selector, unsigned numBytes, uint8_t *peeker, bool reversed ) {
  auto actual = ReadFrom(selector, numBytes);
  if (actual != numBytes) {
    return false;
  }

  if (numBytes == 1) {//expedite common case, reversed is moot
    *peeker = next();
    return true;
  }

  for (unsigned bc = numBytes; bc-- > 0;) {
    uint8_t bite = next();
    if (reversed) {
      peeker[bc] = bite;
    } else {
      *peeker++ = bite;
    }
  }
  return true;
}
